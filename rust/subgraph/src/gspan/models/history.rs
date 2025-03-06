use crate::gspan::models::edge::Edge;
use crate::gspan::models::prev_dfs::PrevDFS;
use rustc_hash::FxHashSet;

/**
 * 在递归函数中，根据当前 DFSCode 构造出的 rmpath 保存了最右路径上的节点索引，我们需要根据这些索引找到原图中最右路径上的边的指针。
 * 利用 PrevDFS.prev 的链表指针向搜索栈上方一个个寻找，可构造出整个图上的边被添加的顺序，该顺序与 DFSCode 中边的排列顺序相同。
 * 这样 History.histories 中就恢复出了按照 DFSCode 的排列形式在对应出现位置上的所有边的指针，即可利用 rmpath 的索引信息直接定位到其出现位置上的边的指针。
 */
pub struct History<'a> {
    pub histories: Vec<&'a Edge>,
    pub edges: FxHashSet<usize>,
    pub vertices: FxHashSet<String>,
}

impl<'a> History<'a> {
    pub fn build(e: &'a PrevDFS<'a>) -> History<'a> {
        let mut history = History {
            histories: Vec::with_capacity(32),
            edges: FxHashSet::default(),
            vertices: FxHashSet::default(),
        };
        let mut e = e;
        loop {
            history.histories.push(e.edge);
            history.edges.insert(e.edge.id);
            history.vertices.insert(e.edge.from.clone());
            history.vertices.insert(e.edge.to.clone());
            if e.prev.is_none() {
                break;
            }
            e = e.prev.as_ref().unwrap()
        }
        history.histories.reverse(); // 翻转成正向的搜索Edge的历史
        history
    }

    pub fn has_edge(&self, id: &usize) -> bool {
        self.edges.contains(&id)
    }

    pub fn has_vertex(&self, name: &str) -> bool {
        self.vertices.contains(name)
    }
}
