use smallvec::SmallVec;

use super::{Edge, Key, EMPTY_KEY};

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum Dummy {
    Root,
    Border,
    Edge,
    EdgeProxy,
    SelfEdge,
}

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum Ranker {
    TightTree,
    LongestPath,
    NetworkSimplex,
}

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum RankDir {
    LR,
    RL,
    TB,
    BT,
}

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum Acyclicer {
    Greedy,
    Dfs,
    NoAcyclicer,
}

#[derive(Debug, Copy, Clone, Default)]
pub struct GraphNode {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
    pub dummy: Option<Dummy>,
    pub rank: Option<i32>,
    pub min_rank: Option<i32>,
    pub max_rank: Option<i32>,
    pub order: Option<usize>,
    pub border_top: Option<Key>,
    pub border_bottom: Option<Key>,
    pub low: Option<usize>,
    pub lim: Option<usize>,
    pub parent: Option<Key>,
    pub edge: Option<Edge>,
}

impl GraphNode {
    pub fn of(x: f32, y: f32, width: f32, height: f32) -> Self {
        Self { x, y, width, height, ..GraphNode::default() }
    }
}

#[derive(Debug, Copy, Clone, Default)]
pub struct Point {
    pub x: f32,
    pub y: f32,
}

impl Point {
    #[inline]
    pub fn of(x: f32, y: f32) -> Self {
        Self { x, y }
    }

    #[inline]
    pub fn from(node: &GraphNode) -> Self {
        Self { x: node.x, y: node.y }
    }
}

#[derive(Debug, Clone)]
pub struct GraphEdge {
    pub source: Key,
    pub target: Key,
    pub reversed: bool,
    pub minlen: Option<i32>,
    pub weight: Option<f32>,
    pub rank: Option<i32>,
    pub nesting: bool,
    pub cutvalue: Option<f32>,
    /// Move this field out of the structure and manage it uniformly,
    /// so GraphEdge can derive the Copy trait
    pub points: Option<SmallVec<Point, 6>>,
}

impl Default for GraphEdge {
    fn default() -> Self {
        Self {
            source: EMPTY_KEY,
            target: EMPTY_KEY,
            reversed: false,
            minlen: Some(1),
            weight: Some(1.0),
            rank: None,
            nesting: false,
            cutvalue: None,
            points: None,
        }
    }
}

impl GraphEdge {
    pub fn of(source: Key, target: Key) -> Self {
        Self { source, target, ..Self::default() }
    }
}
