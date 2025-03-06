use ahash::HashMap;
use serde::{Serialize, Serializer, ser::SerializeStruct};
use smartstring::alias::String;

use self::AttrValue::*;

#[derive(Debug, Serialize)]
pub struct Model {
    pub name: String,
    pub nodes: HashMap<String, Node>,
    pub edges: Vec<(String, String)>,
    pub parameters: HashMap<String, String>,
}

#[allow(non_snake_case)]
#[derive(Debug, Serialize)]
pub struct Node {
    pub name: String,
    pub opType: String,
    pub input: Vec<String>,
    pub output: Vec<String>,
    pub attributes: HashMap<String, AttrValue>,
    pub dynamic: bool
}

#[derive(Debug)]
pub enum AttrValue {
    StringLike(String),
    StringLikeArray(Vec<String>),
    TensorVal(String),
    TensorVals(Vec<String>),
    TensorsTuple(Vec<Vec<String>>),
}

impl Serialize for AttrValue {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut state = serializer.serialize_struct("AttrValue", 2)?;
        match self {
            StringLike(s) => {
                state.serialize_field("type", "string-like")?;
                state.serialize_field("value", s)?;
            }
            StringLikeArray(s) => {
                state.serialize_field("type", "string-like-array")?;
                state.serialize_field("value", s)?;
            }
            TensorVal(s) => {
                state.serialize_field("type", "tensor-val")?;
                state.serialize_field("value", s)?;
            }
            TensorVals(v) => {
                state.serialize_field("type", "tensor-vals")?;
                state.serialize_field("value", v)?;
            }
            TensorsTuple(vv) => {
                state.serialize_field("type", "tensors-tuple")?;
                state.serialize_field("value", vv)?;
            }
        }
        state.end()
    }
}

#[derive(Debug, Serialize)]
pub struct Edge {
    pub source: String,
    pub target: String,
}

impl Edge {
    pub fn new(s: String, t: String) -> Self {
        Self { source: s, target: t }
    }
}
