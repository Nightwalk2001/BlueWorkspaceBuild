use rustc_hash::FxHashSet;

use crate::gspan::models::edge::Edge;

//保存edge数据的链表节点
//结构如，e1->e2->e3->e4->e1->...
//每个链表节点可以表示一个子图

/**
 * PrevDFS的数据结构是一个链表，其本质代表了深度优先搜索中，DFSCode在搜索占中在某一个出现位置的投影。
 * 由于每个child DFSCode都是在parent DFSCode 上增加一条边的结果，如果将每个图或每个图的DFSCode保存在搜索栈中就会浪费大量空间。
 * 因此当前栈中只保存增加的边即PrevDFS.edge，运行时根据PrevDFS.edge的链表指针向前寻找，即可构造出该DFSCode每一条边的添加顺序。
 */
#[derive(Debug)]
pub struct PrevDFS<'a>{
    pub gid: usize, //ID===GSpan.trans上的索引
    pub edge: & 'a Edge, 
    pub prev: Option<Box<&'a PrevDFS<'a>>>,
}

impl<'a> PrevDFS<'a>{
    pub fn new(gid: usize,edge:&'a Edge,prev:Option<&'a PrevDFS<'a>>)->PrevDFS<'a>{
        PrevDFS{
            gid,
            edge,
            prev:match prev{
                Some(prev)=>Some(Box::new(prev)),
                None=>None,
            },
        }
    }

    pub fn get_vertex_names(&self) -> FxHashSet<(usize,String)> {
        let mut names:  FxHashSet<(usize,String)> = FxHashSet::default();

        let mut cur = self;

        loop{
            names.insert((cur.gid.clone(), cur.edge.from.clone()));
            names.insert((cur.gid.clone(), cur.edge.to.clone()));

            if let Some(prev) = &cur.prev {
                cur = **prev;
            }else{
                break;
            }
        }

        names
    }

    pub fn get_edges(&self) -> Vec<&Edge> {
        let mut edges: Vec<&Edge> = vec![];

        let mut cur = self;

        loop{
            edges.push(&cur.edge);

            if let Some(prev) = &cur.prev{
                cur = **prev;
            }else{
                break;
            }
        }
        edges
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_equal_dfs() {
        let edge1 = Edge::new(
            String::from("node_1"),
            String::from("node_2"),
            String::from("a"),
            String::from("b"),
            Some(String::from("A")),
        );
        let edge2 = Edge::new(
            String::from("node_1"),
            String::from("node_2"),
            String::from("a"),
            String::from("b"),
            Some(String::from("A")),
        );
        let edge3 = Edge::new(
            String::from("node_2"),
            String::from("node_3"),
            String::from("a"),
            String::from("b"),
            Some(String::from("C")),
        );

        let prev1 = PrevDFS::new(0,&edge1,None);
        let prev2 = PrevDFS::new(0,&edge2,Some(&prev1));
        let prev3 = PrevDFS::new(0,&edge3,Some(&prev2));

        let result1: FxHashSet<(usize,String)> = vec![(0,String::from("node_1")), (0,String::from("node_2"))]
            .into_iter()
            .collect();
        let result2: FxHashSet<(usize,String)> = vec![
            (0,String::from("node_1")),
            (0,String::from("node_2")),
            (0,String::from("node_3"))
        ]
        .into_iter()
        .collect();

        assert_eq!(result2,prev3.get_vertex_names());
        assert_eq!(result1,prev2.get_vertex_names());
        assert_eq!(result1,prev1.get_vertex_names());
    }
}

