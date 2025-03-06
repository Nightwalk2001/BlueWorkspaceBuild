use std::{cmp, cmp::Ordering};

use super::{Context, ResolvedEntry, Subgraph};
use crate::Key;

impl Context {
    pub fn next_subgraph(&mut self, bias_right: bool) -> Option<Subgraph> {
        let (mut sortable, mut unsortable) = self
            .entries
            .iter()
            .map(|(&k, &e)| (k, e))
            .partition::<Vec<(Key, ResolvedEntry)>, _>(|(_, ent)| ent.barycenter.is_some());

        sortable.sort_by(|(_, lhs), (_, rhs)| cmp_with_bias(lhs, rhs, bias_right));
        unsortable.sort_by_key(|(_, entry)| cmp::Reverse(entry.idx));

        let mut sum = 0.0;
        let mut weight = 0.0;

        self.consume(&mut sortable);

        for (k, entry) in sortable {
            let keys = &self.keys_map[&k];
            self.index += keys.len();
            self.keys.extend(keys);
            let entry_weight = entry.weight?;
            sum += entry.barycenter? * entry_weight;
            weight += entry_weight;
            self.consume(&mut unsortable);
        }

        let mut subgraph = Subgraph::default();
        let mut keys = vec![];
        keys.extend(&self.keys);
        subgraph.keys = keys;
        if weight > 0.0 {
            subgraph.barycenter = sum / weight;
            subgraph.weight = weight;
        }

        Some(subgraph)
    }

    fn consume(&mut self, consumable: &mut Vec<(Key, ResolvedEntry)>) -> Option<()> {
        if consumable.is_empty() {
            return None;
        }

        while let Some((last, _)) = consumable.last() {
            let idx = self.entries[last].idx;
            if idx > self.index as u16 {
                break;
            }

            let keys = &self.keys_map[last];

            self.keys.extend(keys);

            consumable.pop();
            self.index += 1;
        }

        None
    }
}

fn cmp_with_bias(lhs: &ResolvedEntry, rhs: &ResolvedEntry, bias: bool) -> Ordering {
    let lb = lhs.barycenter;
    let rb = rhs.barycenter;
    if lb < rb {
        return Ordering::Greater;
    } else if lb > rb {
        return Ordering::Less;
    }

    if !bias { lhs.idx.cmp(&rhs.idx) } else { rhs.idx.cmp(&lhs.idx) }
}
