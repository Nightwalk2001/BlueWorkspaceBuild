mod onnx {
    include!(concat!(env!("OUT_DIR"), "/onnx.rs"));
}

use ahash::{HashMap, HashMapExt};
use onnx::{
    AttributeProto, GraphProto, ModelProto, SparseTensorProto, TensorProto,
    attribute_proto::AttributeType::*,
};
use smartstring::alias::String;

use super::{TensorFormatter, format_tensors, parse_pb};
use crate::{AttrValue, AttrValue::*, Model, Node, SmartStringExt};

pub fn parse_onnx_model(path: &str) -> Option<Model> {
    match parse_pb::<ModelProto>(path) {
        Ok(model) => model.into(),
        Err(_) => None,
    }
}

impl From<ModelProto> for Option<Model> {
    fn from(value: ModelProto) -> Self {
        if let Some(graph) = value.graph {
            let name = String::from(&graph.name);
            let mut nodes = HashMap::new();
            let parameters = HashMap::new();
            let ops = &graph.node;

            let mut node_ids = Vec::new();
            let mut edges = Vec::new();

            for (idx, op) in ops.iter().enumerate() {
                let op_type = String::from(&op.op_type);
                let node_id: String = format!("{}_{}", op_type, idx).into();
                let input = String::from_slice(&op.input);
                let output = String::from_slice(&op.output);

                /// The repeated field is mapped to Vec<T> regardless of whether it
                /// has a value or not.
                /// However, sometimes the array is actually empty.
                ///
                /// ## Note
                /// Neither HashMap::new nor Vec::new perform memory allocation.
                /// Memory allocation only be performed when there are elements.
                /// Therefore, if attrs is empty, there's actually only one if judgment,
                /// so there's no need
                /// to perform length judgment at the beginning.
                let mut flag = false;
                let mut dyn_shape = false;
                let mut attributes = HashMap::new();
                for attr in &op.attribute {
                    if let Some(value) = parse_attr(attr) {
                        attributes.insert(String::from(&attr.name), value);
                    }

                    if !flag && is_dyn_shape(attr) {
                        flag = true;
                        dyn_shape = true
                    }
                }

                let node = Node {
                    name: node_id.clone(),
                    opType: op_type,
                    input,
                    output,
                    attributes,
                    dynamic: dyn_shape,
                };

                nodes.insert(node_id.clone(), node);
                node_ids.push(node_id.clone());

                for out_tensor in &op.output {
                    for (next_idx, next_op) in ops.iter().enumerate() {
                        if next_op.input.contains(out_tensor) && idx != next_idx {
                            let target_node_id = format!("{}_{}", next_op.op_type, next_idx).into();
                            edges.push((node_id.clone(), target_node_id));
                        }
                    }
                }
            }

            return Some(Model { name, nodes, edges, parameters });
        }

        None
    }
}

/// ## Note
/// One possible optimization isn't to use libraries such as [`prost`] and
/// [`protobuf`] to generate type definitions and parsing files.
/// ## Prost Generated Struct
/// ```
/// use parser::onnx::{GraphProto, SparseTensorProto, TensorProto};
///
/// pub struct AttributeProto {
///     pub r#type: i32,
///     pub f: f32,
///     pub i: i64,
///     pub s: Vec<u8>,
///     pub t: Option<TensorProto>,
///     pub g: Option<GraphProto>,
///     pub sparse_tensor: Option<SparseTensorProto>,
///     pub floats: Vec<f32>,
///     pub ints: Vec<i64>,
///     pub strings: Vec<Vec<u8>>,
///     pub tensors: Vec<TensorProto>,
///     pub graphs: Vec<GraphProto>,
///     pub sparse_tensors: Vec<SparseTensorProto>,
/// }
/// ```
/// The [`type`] field is a discriminator, which indicates which field the data is stored in.
/// Therefore, the equivalent is actually a [`union`] rather than a [`struct`].
/// The preceding structure wastes a lot of memory.
fn parse_attr(attr: &AttributeProto) -> Option<AttrValue> {
    let discriminator = attr.r#type();

    let value = match discriminator {
        Undefined => StringLike(String::from("")),
        Float => StringLike(String::from_f32(attr.f)),
        Int => StringLike(String::from_i64(attr.i)),
        String => StringLike(String::from_vecu8(&attr.s)),
        /// ## Safety
        /// These fields are marked as [`optional`], otherwise, during parsing
        /// phase, they were filled with [`Default::default`] even if they
        /// actually have no value, causing unnecessary memory allocation.
        /// When the [`discriminator`] corresponds, the [`Option`] is guaranteed
        /// to have a value, so the [`unwrap`] here is safe
        ///
        /// ## Panics
        /// If the file is corrupted,
        /// [`unwrap`] here panics, causing the program to exit.
        Tensor => StringLike(TensorFormatter::fmt(attr.t.as_ref()?)),
        Graph => StringLike(format_graph(attr.g.as_ref()?)),
        SparseTensor => StringLike(String::from_i64s(&attr.sparse_tensor.as_ref()?.dims)),
        Floats => StringLike(String::from_f32s(&attr.floats)),
        Ints => StringLike(String::from_i64s(&attr.ints)),
        Strings => StringLike(String::from_2dvecu8(&attr.strings)),
        Tensors => format_tensors(&attr.tensors),
        Graphs => StringLike(format_graphs(&attr.graphs)),
        SparseTensors => format_tensors(&attr.sparse_tensors),
    };

    Some(value)
}

fn is_dyn_shape(attr: &AttributeProto) -> bool {
    attr.i == 1 && (attr.name == "_is_unknown_shape" || attr.name == "_force_unknown_shape")
}

impl TensorFormatter for TensorProto {
    fn fmt(&self) -> String {
        let dtype = self.dtype();
        let dims = String::from_i64s(&self.dims);

        format!("{:?}({})", dtype, dims).into()
    }
}

impl TensorFormatter for SparseTensorProto {
    fn fmt(&self) -> String {
        String::from_i64s(&self.dims)
    }
}

/// ## Note
/// Temporarily displayed as a string
fn format_graph(graph: &GraphProto) -> String {
    format!("Graph({})", graph.name).into()
}

fn format_graphs(graphs: &[GraphProto]) -> String {
    let mut s = String::from("Graphs[");
    let last_idx = graphs.len() - 1;
    for (i, g) in graphs.iter().enumerate() {
        s.push_str(&g.name);
        if i < last_idx {
            s.push(';');
        }
    }

    s.push(']');

    s
}
