// DFS ∈ Edge， 是没有 edge id 的五元组
// 但要注意的是，DFS 上的 from to 和 Edge 上的 from to 无关
// DFS 是子图的五元组，Edge 是完整图的五元组
#[derive(PartialEq, Debug, Clone)]
pub struct DFS {
    pub from: usize,
    pub to: usize,
    pub from_label: String,
    pub e_label: String,
    pub to_label: String,
}

impl DFS {
    // pub fn new() -> DFS {
    //     DFS {
    //         from: 0,
    //         to: 0,
    //         from_label: String::new(),
    //         e_label: String::new(),
    //         to_label: String::new(),
    //     }
    // }

    pub fn from(
        from: usize,
        to: usize,
        from_label: String,
        e_label: String,
        to_label: String,
    ) -> DFS {
        DFS {
            from,
            to,
            from_label,
            e_label,
            to_label,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_equal_dfs() {
        let dfs1 = DFS::from(
            1,
            2,
            String::from("3"),
            String::from("4"),
            String::from("5"),
        );
        let dfs2 = DFS::from(
            1,
            2,
            String::from("3"),
            String::from("4"),
            String::from("5"),
        );
        let dfs3 = DFS::from(
            2,
            2,
            String::from("3"),
            String::from("4"),
            String::from("5"),
        );

        assert_eq!(dfs1, dfs2);
        assert_ne!(dfs1, dfs3);
        assert_ne!(dfs2, dfs3);
    }
}
