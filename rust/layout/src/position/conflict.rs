use super::Context;
use crate::{normalize_st, Dummy::Border, Graph, Key};

#[derive(Copy, Clone)]
struct ScanCtx {
    south_start: usize,
    south_end: usize,
    prev_north: i32,
    next_north: i32,
}

impl ScanCtx {
    fn of(south_start: usize, south_end: usize, prev_north: i32, next_north: i32) -> Self {
        Self { south_start, south_end, prev_north, next_north }
    }
}

impl Graph {
    pub(super) fn find_conflict(&mut self, ctx: &mut Context, matrix: &[Vec<Key>]) {
        if !matrix.is_empty() {
            self.find_type1_conflict(ctx, matrix);
            self.find_type2_conflict(ctx, matrix);
        }
    }
    
    fn find_type1_conflict(&mut self, ctx: &mut Context, matrix: &[Vec<Key>]) -> Option<()> {
        let mut prev_len = matrix.first()?.len();

        for layer in matrix.iter().skip(1) {
            let mut k0 = 0;
            let mut scan_pos = 0;
            let last_key = layer.last()?;

            for (i, key) in layer.iter().enumerate() {
                let w = self.other_inner_segment(key);

                let k1 = match (w, key == last_key) {
                    (None, false) => continue,
                    (Some(w), _) => self.node(&w)?.order?,
                    (None, true) => prev_len,
                };

                for scan_key in &layer[scan_pos..i + 1] {
                    let scan_node = self.node(scan_key)?;
                    for pre_key in &self.predecessors(scan_key) {
                        let pre_node = self.node(pre_key)?;
                        let pos = pre_node.order?;
                        let both_dummy = pre_node.dummy.is_some() && scan_node.dummy.is_some();
                        if (pos < k0 || k1 < pos) && !both_dummy {
                            ctx.add_conflict(*pre_key,*scan_key)
                        }
                    }
                }

                scan_pos = i + 1;
                k0 = k1;
                prev_len = layer.len();
            }
        }

        None
    }

    fn find_type2_conflict(&mut self, ctx: &mut Context, matrix: &[Vec<Key>]) -> Option<()> {
        let mut north_len = matrix.first()?.len();

        for south in matrix.iter().skip(1) {
            let mut prev_north_pos = -1;
            let mut next_north_pos = 0;
            let mut south_pos = 0;
            let south_len = south.len();

            for (south_ahead, key) in south.iter().enumerate() {
                if self.node(key)?.dummy == Some(Border) {
                    let preds = self.predecessors(key);
                    if let [pk] = preds[..] {
                        next_north_pos = self.node(&pk)?.order? as i32;
                        let scan =
                            ScanCtx::of(south_pos, south_ahead, prev_north_pos, next_north_pos);
                        self.scan(ctx, south, scan);
                        south_pos = south_ahead;
                        prev_north_pos = next_north_pos;
                    }
                }

                let scan = ScanCtx::of(south_pos, south_len, next_north_pos, north_len as i32);
                self.scan(ctx, south, scan);
            }

            north_len = south_len;
        }

        None
    }

    fn scan(&mut self, ctx: &mut Context, south: &[Key], scan_ctx: ScanCtx) -> Option<()> {
        let ScanCtx {
            south_start,
            south_end,
            prev_north: prev_north_border,
            next_north: next_north_border,
        } = scan_ctx;
        for sid in &south[south_start..south_end] {
            if self.node(sid)?.dummy.is_none() {
                continue;
            }

            for id in self.predecessors(sid) {
                let Some(node) = self.node(&id) else { continue };

                let order = node.order.unwrap_or(0) as i32;
                let has_conflict = order < prev_north_border || order > next_north_border;
                if node.dummy.is_some() && has_conflict {
                    ctx.add_conflict(id,*sid);
                }
            }
        }

        None
    }

    fn other_inner_segment(&mut self, key: &Key) -> Option<Key> {
        match self.node(key)?.dummy {
            Some(_) => {
                /// ### Functional style
                /// ```asm
                /// .iter().find(|u| g.node(u).unwrap().dummy.is_some()).copied()
                /// ```
                /// However, there's [`unwrap`] and [`copied`]
                for k in self.predecessors(key) {
                    if self.node(&k)?.dummy.is_some() {
                        return Some(k);
                    }
                }

                None
            }
            None => None,
        }
    }
}

impl Context {
    fn add_conflict(&mut self, source: Key, target: Key) {
        let (s, t) = normalize_st(source, target);

        self.conflicts.entry(s).or_default().push(t);
    }

    pub(super) fn has_conflict(&self, source: Key, target: Key) -> bool {
        let (s, t) = normalize_st(source, target);

        match self.conflicts.get(&s) {
            Some(set) => set.contains(&t),
            _ => false,
        }
    }
}
