use ahash::{HashMap, HashMapExt};
use layout::{layout, Graph, GraphEdge, GraphNode, Key, KeyCodecExt};
use parser::{parse_geir_model, parse_mindir_model, parse_onnx_model, Model};
use serde::Serialize;
use smartstring::alias::String;

use crate::geometry::{calc_edge_bounding, line_curve, Point};

#[allow(non_snake_case)]
#[derive(Serialize)]
struct RenderNode {
    id: String,
    x: i32,
    y: i32,
    width: i32,
    height: i32,
    opType: String,
}

impl RenderNode {
    fn new(id: String, node: &GraphNode, op_type: String) -> Self {
        let x = (node.x - node.width / 2.0).round() as i32;
        let y = (node.y - node.height / 2.0).round() as i32;
        let width = node.width as i32;
        let height = node.height as i32;

        Self { id, x, y, width, height, opType: op_type }
    }
}

#[derive(Serialize)]
struct RenderEdge {
    x: i32,
    y: i32,
    width: i32,
    height: i32,
    source: String,
    target: String,
    points: Vec<Point>,
    path: String,
}

impl RenderEdge {
    fn new(source: String, target: String, edge: &GraphEdge) -> Self {
        let mut points = Vec::new();

        if let Some(ps) = &edge.points {
            for p in ps {
                points.push(Point::from(p));
            }
        }

        let path = line_curve(&points);
        let (x, y, width, height) = calc_edge_bounding(&points);

        Self { x, y, width, height, source, target, points, path }
    }
}

fn layout_model(model: &Model) -> (Vec<RenderNode>, Vec<RenderEdge>) {
    let mut g = Graph::default();

    let mut counter: Key = 0;

    let mut name_key = HashMap::new();
    let mut key_name = HashMap::new();

    for key in model.nodes.keys() {
        name_key.insert(key.clone(), counter);
        key_name.insert(counter, key.clone());
        counter += 1;
    }

    for (source, target) in &model.edges {
        if let (Some(&s), Some(&t)) = (name_key.get(source), name_key.get(target)) {
            g.set_edge(s, t, Some(GraphEdge::of(s, t)));
        }
    }

    for (id, node) in &model.nodes {
        let key = name_key[id];
        g.set_node(key, Some(GraphNode::of(0.0, 0.0, 100.0, 30.0)));

        for source in &node.input {
            if let Some(&source) = name_key.get(source) {
                g.set_edge(source, key, Some(GraphEdge::of(source, key)));
            }
        }

        for sink in &node.output {
            if let Some(&sink) = name_key.get(sink) {
                g.set_edge(key, sink, Some(GraphEdge::of(key, sink)));
            }
        }
    }

    layout(&mut g);

    let mut nodes = Vec::with_capacity(g.nodes.len());
    let mut edges = Vec::with_capacity(g.edge_values.len());

    for (id, node) in &g.nodes {
        let node_id = &key_name[id];
        let op_type = model.nodes[node_id].opType.clone();
        let node = RenderNode::new(node_id.clone(), node, op_type);
        nodes.push(node);
    }

    for (&id, edge) in &g.edge_values {
        let (s, t) = id.decode();
        let source = key_name[&s].clone();
        let target = key_name[&t].clone();

        let edge = RenderEdge::new(source, target, edge);
        edges.push(edge);
    }

    (nodes, edges)
}

#[derive(Serialize)]
pub struct LayoutRet {
    model: Model,
    nodes: Vec<RenderNode>,
    edges: Vec<RenderEdge>,
}

macro_rules! layout_command {
    ($func_name:ident, $parse_func:ident) => {
        pub fn $func_name(path: &str) -> Option<LayoutRet> {
            let model = $parse_func(path)?;
            let (nodes, edges) = layout_model(&model);
            Some(LayoutRet { model, nodes, edges })
        }
    };
}

layout_command!(layout_mindir, parse_mindir_model);
layout_command!(layout_geir, parse_geir_model);
layout_command!(layout_onnx, parse_onnx_model);
