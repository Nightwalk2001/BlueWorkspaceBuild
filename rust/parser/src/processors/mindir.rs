mod mindir {
    include!(concat!(env!("OUT_DIR"), "/mindir.rs"));
}

use ahash::{HashMap, HashMapExt, HashSet, HashSetExt};
use mindir::{
    AttributeProto, GraphProto, ModelProto, NodeProto, PrimitiveProto, TensorProto,
    attribute_proto::AttributeType,
};
use smartstring::alias::String;

use super::{TensorFormatter, format_tensors, parse_pb};
use crate::{AttrValue, AttrValue::*, Model, Node, SmartStringExt, StrExt};

pub fn parse_mindir_model(path: &str) -> Option<Model> {
    match parse_pb::<ModelProto>(path) {
        Ok(model_proto) => model_proto.into(),
        _ => None,
    }
}

impl From<ModelProto> for Option<Model> {
    fn from(model: ModelProto) -> Self {
        if let Some(graph) = model.graph {
            let name = String::from(graph.name());

            let mut ctx = Context::new(name.clone());

            ctx.prepare(&model.primitives);
            ctx.process_graph(graph);

            return Some(Model {
                name,
                nodes: ctx.nodes,
                edges: vec![],
                parameters: ctx.parameters,
            });
        }

        None
    }
}

struct Context {
    prefix: String,
    nodes: HashMap<String, Node>,
    node_names: HashSet<String>,
    op_types: HashSet<String>,
    node_name_map: HashMap<String, String>,
    parameters: HashMap<String, String>,
}

impl Context {
    fn new(name: String) -> Context {
        let prefix = format!("{}:", name).into();

        let nodes = HashMap::new();
        let op_types = HashSet::new();
        let node_names = HashSet::new();
        let node_name_map = HashMap::new();
        let parameters = HashMap::new();

        Context { prefix, nodes, node_names, op_types, node_name_map, parameters }
    }

    #[inline]
    fn prefix(&self) -> &str {
        &self.prefix
    }

    fn prepare(&mut self, primitives: &[PrimitiveProto]) {
        for primitive in primitives {
            if !primitive.attribute.is_empty() {
                self.op_types.insert(format!("REF::{}", primitive.name()).into());
            }
        }
    }

    fn process_graph(&mut self, graph: GraphProto) {
        for parameter in &graph.parameter {
            let s = parameter.name.strip_prefix1(self.prefix());
            self.parameters.insert(s.clone(), parameter.fmt());
            self.node_names.insert(s);
        }

        for source in &graph.input {
            self.node_names.insert(source.name.strip_prefix1(self.prefix()));
        }

        for output in &graph.output {
            self.node_names.insert(output.name.strip_prefix1(self.prefix()));
        }

        for node in &graph.node {
            self.process_node(node);
        }
    }

    fn process_node(&mut self, node: &NodeProto) -> Option<()> {
        if let Some(op_type) = node.op_type1(self) {
            let node_name = node.stripped_name(self.prefix());

            let (input, output, attributes) = node.extract_details(self)?;

            self.nodes.insert(node_name.clone(), Node {
                name: node_name,
                opType: op_type,
                input,
                output,
                attributes,
                dynamic: false,
            });
        }

        None
    }
}

impl NodeProto {
    #[inline]
    fn stripped_name(&self, prefix: &str) -> String {
        self.name.strip_prefix1(prefix)
    }

    #[inline]
    fn op_type1(&self, ctx: &mut Context) -> Option<String> {
        let op: String = String::from(self.op_type());
        match ctx.op_types.contains(&op) {
            true => Some(strip_op(op)),
            _ => {
                self.when_op_missing(ctx);
                None
            }
        }
    }

    fn extract_details(
        &self,
        ctx: &mut Context,
    ) -> Option<(Vec<String>, Vec<String>, HashMap<String, AttrValue>)> {
        let mut input = vec![];
        let mut output = vec![];
        let mut attributes = HashMap::new();

        for attr in &self.attribute {
            if attr.name.is_none() {
                continue;
            }
            let ret = parse_attr(attr)?;
            attributes.insert(String::from(attr.name()), ret);
        }

        ctx.node_names.insert(self.stripped_name(ctx.prefix()));

        for source in &self.input {
            if source == &self.name() {
                continue;
            }
            let mut s = source.strip_prefix1(ctx.prefix());
            if !ctx.node_names.contains(&s) {
                s = ctx.node_name_map.get(&s).cloned().unwrap_or_default();
            }
            (!s.is_empty()).then(|| input.push(s));
        }

        for sink in &self.output {
            if sink == &self.name() {
                continue;
            }
            let s = sink.strip_prefix1(ctx.prefix());
            output.push(s);
        }

        Some((input, output, attributes))
    }

    fn when_op_missing(&self, ctx: &mut Context) {
        for source in &self.input {
            let s = source.strip_prefix1(ctx.prefix());
            if ctx.node_names.contains(&s) {
                ctx.node_name_map.insert(self.stripped_name(ctx.prefix()), s);
            }
        }
    }
}

fn parse_attr(attr: &AttributeProto) -> Option<AttrValue> {
    use AttributeType::{
        Bfloat16, Complex64, Complex128, Double, Float, Float16, Int8, Int16, Int32, Int64, Tensor,
        Tensors, Tuple, TypeNull, Uint8, Uint16, Uint32, Uint64, Undefined,
    };

    let discriminator = attr.r#type();

    /// Disrupted the order and put the most common cases in front
    let value = match discriminator {
        Tensor => StringLike(attr.t.as_ref()?.fmt()),
        Tensors => format_tensors(&attr.tensors),
        Tuple => format_tuple(&attr.values),
        AttributeType::String => StringLike(String::from_vecu8(attr.s())),
        Undefined | AttributeType::None | TypeNull => StringLike(String::from("Undefined")),
        Double => StringLike(String::from_f64(attr.d())),
        Float => StringLike(String::from_f32(attr.f())),
        Float16 => StringLike(format!("f16({})", String::from_f32(attr.f())).into()),
        Bfloat16 => StringLike(format!("bf16({})", String::from_f32(attr.f())).into()),
        Uint8 | Int8 | Uint16 | Int16 | Uint32 | Int32 | Uint64 | Int64 => StringLike("".into()),
        Complex64 | Complex128 => StringLike("".into()),
        /// ```ts
        /// Bool => "",
        /// List => "",
        /// Dict => "",
        /// Umonad => "",
        /// Iomonad => "",
        /// Primitiveclosure => "",
        /// Funcgraphclosure => "",
        /// Partialclosure => "",
        /// Unionfuncclosure => "",
        /// CsrTensor => "",
        /// CooTensor => "",
        /// RowTensor => "",
        /// ClassType => "",
        /// NameSpace => "",
        /// Symbol => "",
        /// MapTensor => "",
        /// Functor => "",
        /// Scalar => "",
        /// ScalarGraphHolder => "",
        /// Graph => "",
        /// ```
        _ => StringLike("".into()),
    };

    Some(value)
}

/// ## Safety
/// If [`op_type`] starts with `REF::`, the following part is [`OP_NAME:index`], so [`unwrap`] is safe.
///
/// ## Panics
/// Binary content is modified
#[inline]
fn strip_op(raw: String) -> String {
    match raw.strip_prefix("REF::") {
        Some(stripped) => stripped.split_once(':').unwrap().0.into(),
        _ => raw,
    }
}

impl TensorFormatter for TensorProto {
    #[inline]
    fn fmt(&self) -> String {
        let dims = String::from_i64s(&self.dims);
        format!("{:?}({})", self.dtype(), dims).into()
    }
}

#[inline]
fn format_tuple(values: &[AttributeProto]) -> AttrValue {
    TensorsTuple(values.iter().map(|ts| ts.tensors.iter().map(|t| t.fmt()).collect()).collect())
}
