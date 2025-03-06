//! ### Implementation Based on the description in Forster.
//! ### "A Fast and Simple Heuristic for Constrained Two-Level Crossing Reduction"
//! Thought differs in some specific details.
//!
//! Given a list of entries with the form {key, barycenter, weight} and a
//! constraint graph, this function resolves any conflicts between the
//! constraint graph and the barycenters for the entries.
//! If the barycenters for an entry violate a constraint in the constraint graph,
//! then coalesce the nodes in the conflict into a new node that respects the
//! constraint and aggregates barycenter and weight information.
//!
//! Pre-Conditions:
//!
//! 1. Each entry has the form {key, barycenter, weight}, or if the node has
//! no barycenter, then {key}.
//!
//! Returns:
//!
//! A new list of entries with the form {keys, idx, barycenter, weight}.
//! The list `keys` may either be a singleton or it may be aggregation of nodes
//! ordered such that they don't violate constraints from the constraint graph.
//! The property `idx` is the lowest original index of the elements in `keys`.

use ahash::HashMap;

use super::{Barycenter, Context, ResolvedEntry, Subgraph};
use crate::{Edge, Graph, Key};

impl Context {
    pub(super) fn resolve(&mut self, cg: &Graph, entries: &[Barycenter]) -> Option<()> {
        for (idx, entry) in entries.iter().enumerate() {
            let mut tmp = ResolvedEntry::of(idx as u16);
            if entry.barycenter.is_some() {
                tmp.barycenter = entry.barycenter;
                tmp.weight = entry.weight;
            }
            self.entries.insert(entry.key, tmp);
            self.keys_map.insert(entry.key, vec![entry.key]);
        }

        for &Edge { source, target } in cg.edges.values() {
            let flag = self.entries.get(&source).is_some();
            let te = self.entries.get_mut(&target);
            if flag && let Some(te) = te {
                te.indegree += 1;
                self.sinks_map.entry(source).or_default().push(target);
            }
        }

        let mut source_array: Vec<Key> = self
            .entries
            .iter()
            .filter(|(k, _)| self.sources_map.get(k).is_none())
            .map(|(&k, _)| k)
            .collect();

        self.do_resolve(&mut source_array)
    }

    fn handle_in(&mut self, source: Key, target: Key) -> Option<()> {
        let source_ent = self.entries[&target];
        if source_ent.merged {
            return None;
        }

        let target_ent = self.entries[&source];

        if source_ent.barycenter.is_none()
            || target_ent.barycenter.is_none()
            || source_ent.barycenter >= target_ent.barycenter
        {
            self.merge_entries(target, source);
        }

        None
    }

    fn handle_out(&mut self, source: Key, target: Key) -> Option<()> {
        self.sources_map.get_mut(&target)?.push(source);

        let target_ent = self.entries.get_mut(&target)?;
        target_ent.indegree -= 1;

        if target_ent.indegree == 0 {
            self.sinks_map.get_mut(&source)?.push(target);
        }

        None
    }

    fn do_resolve(&mut self, keys: &mut Vec<Key>) -> Option<()> {
        while let Some(key) = keys.pop() {
            for &source_key in self.sources_map[&key].clone().iter().rev() {
                self.handle_in(source_key, key);
            }

            for sink_key in self.sinks_map.get(&key).cloned()? {
                self.handle_out(key, sink_key);
            }
        }

        None
    }

    fn merge_entries(&mut self, source_key: Key, target_key: Key) -> Option<()> {
        let mut sum = 0.0;
        let mut weight = 0.0;

        let source = &self.entries[&source_key];
        if let Some(w) = source.weight {
            let source_barycenter = source.barycenter?;
            sum += source_barycenter * w;
            weight += w;
        }
        let source_idx = source.idx;

        let target = &self.entries[&target_key];
        if let Some(w) = target.weight {
            let target_barycenter = target.barycenter?;
            sum += target_barycenter * w;
            weight += w;
        }

        let source_keys = self.keys_map[&target_key].clone();
        let target_keys = self.keys_map.get_mut(&source_key)?;
        target_keys.extend(source_keys);

        let target = self.entries.get_mut(&target_key)?;
        target.barycenter = Some(sum / weight);
        target.weight = Some(weight);
        target.idx = source_idx.min(target.idx);

        self.entries.get_mut(&source_key)?.merged = true;

        None
    }

    #[inline]
    pub(super) fn expand_subgraph(&mut self, subgraphs: &HashMap<Key, Subgraph>) {
        for (key, keys) in &mut self.keys_map {
            if let Some(subgraph) = subgraphs.get(key) {
                keys.extend(&subgraph.keys);
            }
        }
    }
}
