mod geir {
    include!(concat!(env!("OUT_DIR"), "/geir.rs"));
}

use ahash::{HashMap, HashMapExt};
use geir::{AttrDef, DataType, ModelDef, TensorDef, TensorDescriptor};
use smartstring::alias::String;

use super::{TensorFormatter, format_tensors, parse_pb};
use crate::{AttrValue, AttrValue::*, Model, Node, SmartStringExt};

impl From<ModelDef> for Option<Model> {
    fn from(value: ModelDef) -> Self {
        if let Some(graph) = value.graph.first() {
            let name = String::from(&graph.name);
            let mut nodes = HashMap::new();
            let parameters = HashMap::new();
            let ops = &graph.op;
            for op in ops {
                let name = String::from(&op.name);
                let op_type = String::from(&op.r#type);
                let mut attributes = HashMap::new();
                let input = String::from_slice(&op.input_name);
                for (name, attr) in &op.attr {
                    if let Some(val) = parse_attr(attr) {
                        attributes.insert(String::from(name), val);
                    }
                }
                let node =
                    Node { name: name.clone(), opType: op_type, input, output: vec![], attributes, dynamic:false };

                nodes.insert(name, node);
            }

            return Some(Model { name, nodes, edges: vec![], parameters });
        }

        None
    }
}

pub fn parse_geir_model(path: &str) -> Option<Model> {
    match parse_pb::<ModelDef>(path) {
        Ok(model) => model.into(),
        Err(_) => None,
    }
}

impl TensorFormatter for TensorDescriptor {
    fn fmt(&self) -> String {
        let dtype = self.dtype();
        let layout = self.layout.as_str();
        if let Some(shape) = &self.shape {
            let dims = String::from_i64s(&shape.dim);
            return format!("{:?}({})【{}】", dtype, dims, layout).into();
        }

        format!("{:?}【{}】", dtype, layout).into()
    }
}

fn parse_attr(attr: &AttrDef) -> Option<AttrValue> {
    use geir::attr_def::Value::*;

    let Some(value) = attr.value.as_ref() else {
        return None;
    };
    let val = match value {
        S(bytes) => StringLike(String::from_vecu8(bytes)),
        I(i) => StringLike(String::from_i64(*i)),
        F(f) => StringLike(String::from_f32(*f)),
        B(b) => StringLike(String::from_bool(*b)),
        /// this branch is likely unreachable
        Bt(bytes) => StringLike(String::from_vecu8(bytes)),
        List(list) => {
            use geir::attr_def::list_value::ListValueType::*;

            match list.val_type() {
                /// refer to **geir.proto**, this variant is unreachable
                VtListNone => unreachable!(),
                VtListString => StringLikeArray(String::vecu8s2strings(&list.s)),
                VtListInt => StringLike(String::from_i64s(&list.i)),
                VtListFloat => StringLike(String::from_f32s(&list.f)),
                VtListBool => StringLike(String::from_bools(&list.b)),
                VtListBytes => StringLikeArray(String::vecu8s2strings(&list.s)),
                VtListTensorDesc => format_tensors(&list.td),
                VtListTensor => format_tensor_defs(&list.t),
                /// unimplemented
                VtListGraph => todo!(),
                /// unimplemented
                VtListNamedAttrs => todo!(),
                VtListDataType => {
                    let vals = list.dt().map(|s| String::from(s.as_str_name())).collect();
                    StringLikeArray(vals)
                }
            }
        }
        /// unimplemented
        Func(_) => return None,
        Td(td) => TensorVal(td.fmt()),
        T(t) => match &t.desc {
            Some(td) => TensorVal(td.fmt()),
            _ => StringLike(String::null()),
        },
        /// unimplemented
        G(_) => return None,
        ListListInt(lli) => {
            StringLikeArray(lli.list_list_i.iter().map(|l| String::from_i64s(&l.list_i)).collect())
        }
        ListListFloat(llf) => {
            StringLikeArray(llf.list_list_f.iter().map(|l| String::from_f32s(&l.list_f)).collect())
        }
        /// used of deprecated api
        Dt(dt) => StringLike(DataType::try_from(*dt).unwrap().as_str_name().into()),
    };

    Some(val)
}

/// maybe t.desc is always Some
fn format_tensor_defs(values: &[TensorDef]) -> AttrValue {
    StringLikeArray(
        values
            .iter()
            .map(|t| match &t.desc {
                Some(t) => t.fmt(),
                _ => String::null(),
            })
            .collect(),
    )
}
