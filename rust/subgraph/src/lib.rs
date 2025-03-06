pub mod gspan;
pub use gspan::*;

pub mod io;
pub use io::*;

pub mod strategy;
pub use strategy::*;

use parser::{parse_geir_model, parse_mindir_model, parse_onnx_model, Model, Node};
use crate::models::graph::Graph;
use crate::result::JSONResult;
use crate::strategy::{config::Config, mining_strategy::MiningStrategy, gspan_mining::GSpanMining};
use crate::gspan::result::OutType;
use crate::io::model_graph::ModelGraph;
use crate::io::node::Node as ModelNode;


impl From<parser::Model> for ModelGraph {
    fn from(model: Model) -> Self {
        ModelGraph {
            name: model.name.to_string(),
            nodes: model.nodes.into_iter().map(|(k, v)| (k.to_string(), v.into())).collect(),
            edges: model.edges.into_iter().map(|(k, v)| (k.to_string(), v.to_string())).collect(),
            parameters: model.parameters.into_iter().map(|(k, v)| (k.to_string(), v.to_string())).collect(),
        }
    }
}

impl From<Node> for ModelNode {
    fn from(node: Node) -> Self {
        ModelNode {
            name: node.name.to_string(),
            opType: node.opType.to_string(),
            input: node.input.into_iter().map(|s| s.to_string()).collect(),
        }
    }
}

macro_rules! subgraph_command {
    ($func_name:ident, $parse_func:ident) => {
        pub fn $func_name(path: &str, min_inner_support: usize, max_vertices: usize) -> Option<Vec<JSONResult>> {
            let raw = $parse_func(path)?;
            let model_graph = ModelGraph::from(raw);

            let graph = Graph::graph_from_model_graph(model_graph, true);

            let gspan_mining = GSpanMining;

            match Config::new_from_graphs(
                vec![graph],
                None,
                None,
                OutType::JSON,
                1,
                min_inner_support,
                2,
                max_vertices,
            ) {
                Ok(config) => {
                    let result = gspan_mining.run(config);
                    Some(result)
                }
                Err(e) => {
                    eprintln!("Failed to create config: {:?}", e);
                    None
                }
            }
        }
    };
}

subgraph_command!(subgraphs_mindir, parse_mindir_model);
subgraph_command!(subgraphs_geir, parse_geir_model);
subgraph_command!(subgraphs_onnx, parse_onnx_model);
