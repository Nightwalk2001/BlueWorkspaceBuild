use super::Barycenter;
use crate::{Graph, Key};

impl Graph {
    pub(super) fn barycenters(&self, movable: &[Key]) -> Vec<Barycenter> {
        movable
            .iter()
            .map(|&key| {
                if self.edge_values.is_empty() {
                    return Barycenter { key, barycenter: None, weight: None };
                }

                let (sum, weight) = self
                    .edge_values
                    .values()
                    .try_fold((0.0, 0.0), |(sum, weight), edge| {
                        let w = edge.weight?;
                        let order = self.node(&edge.source)?.order? as f32;
                        Some((sum + w * order, weight + w))
                    })
                    .unwrap();

                return Barycenter { key, barycenter: Some(sum / weight), weight: Some(weight) };
            })
            .collect()
    }
}
