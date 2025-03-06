use super::{Acyclicer, Acyclicer::NoAcyclicer, RankDir, RankDir::TB, Ranker, Ranker::TightTree};

#[derive(Debug, Copy, Clone)]
pub struct GraphConfig {
    pub nodesep: f32,
    pub edgesep: f32,
    pub ranksep: f32,
    pub rankdir: RankDir,
    pub acyclicer: Acyclicer,
    pub ranker: Ranker,
    pub node_rank_factor: f32,
}

impl Default for GraphConfig {
    fn default() -> Self {
        Self {
            nodesep: 20.0,
            edgesep: 20.0,
            ranksep: 20.0,
            rankdir: TB,
            acyclicer: NoAcyclicer,
            ranker: TightTree,
            node_rank_factor: 0.0,
        }
    }
}
