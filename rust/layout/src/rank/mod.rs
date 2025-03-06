mod feasible_tree;
mod longest_path;
mod network_simplex;
mod slack;

use crate::{Graph, Ranker::*};

impl Graph {
    #[inline]
    pub(super) fn rank(&mut self) {
        match self.config.ranker {
            NetworkSimplex => self.network_simplex(),
            TightTree => self.tight_tree(),
            LongestPath => self.longest_path(),
        }
    }

    #[inline]
    fn tight_tree(&mut self) {
        self.longest_path();
        self.feasible_tree();
    }
}
