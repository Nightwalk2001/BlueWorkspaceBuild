use rustc_hash::FxHashSet;
use crate::gspan::misc::{
    get_backward, get_forward_edges, get_forward_pure, get_forward_rm_path, inner_support, support,
};
use crate::gspan::models::dfs_code::DFSCode;
use crate::gspan::models::edge::Edge;
use crate::gspan::models::graph::Graph;
use crate::gspan::models::history::History;
use crate::gspan::models::projected::Projected;
use crate::gspan::models::vertex::Vertex;
use crate::gspan::result::MaxDFSCodeGraphResult;
use std::collections::BTreeMap;
use std::fs::File;
use std::io::{BufWriter, Write};
use std::usize;

use super::result::OutType;

pub struct GSpan {
    trans: Vec<Graph>,      // 图列表
    min_sup: usize,         // Min support, 相同结果在不同图中出现的最小次数
    inner_min_sup: usize,   // 相同结构在同一图中出现的最小次数
    max_pat_min: usize,     // Minimum number of patterns(vertices) to be output
    max_pat_max: usize,     // Maximum number of patterns(vertices) to be output
    directed: bool,         // 是否有向图
}

impl GSpan {
    pub fn new(
        graphs: Vec<Graph>,
        min_sup: usize,
        inner_min_sup: usize,
        max_pat_min: usize,
        max_pat_max: usize,
        directed: bool,
        out_type: OutType,
    ) -> GSpan {
        let singleton = MaxDFSCodeGraphResult::get_instance();
        singleton.set_config(min_sup, inner_min_sup, max_pat_min, max_pat_max, out_type);
        GSpan {
            trans: graphs,
            min_sup,
            inner_min_sup,
            max_pat_min,
            max_pat_max,
            directed,
        }
    }

    pub fn new_with_out_path(
        graphs: Vec<Graph>,
        min_sup: usize,
        inner_min_sup: usize,
        max_pat_min: usize,
        max_pat_max: usize,
        directed: bool,
        out_path: &str,
        out_type: OutType,
    ) -> GSpan {
        let singleton = MaxDFSCodeGraphResult::get_instance();
        singleton.set_config(min_sup, inner_min_sup, max_pat_min, max_pat_max, out_type);
        singleton.set_stream(BufWriter::new(File::create(out_path).unwrap()));
        GSpan {
            trans: graphs,
            min_sup,
            inner_min_sup,
            max_pat_min,
            max_pat_max,
            directed,
        }
    }

    #[allow(dead_code)]
    pub fn new_with_stream<W: Write + Send + Sync + 'static>(
        graphs: Vec<Graph>,
        min_sup: usize,
        inner_min_sup: usize,
        max_pat_min: usize,
        max_pat_max: usize,
        directed: bool,
        output: W,
        out_type: OutType,
    ) -> GSpan {
        let singleton = MaxDFSCodeGraphResult::get_instance();
        singleton.set_config(min_sup, inner_min_sup, max_pat_min, max_pat_max, out_type);
        singleton.set_stream(output);
        GSpan {
            trans: graphs,
            min_sup,
            inner_min_sup,
            max_pat_min,
            max_pat_max,
            directed,
        }
    }

    pub fn run<W: Write + Send + Sync + 'static>(&self, process: &mut Option<W>) -> usize {
        // 1. Find single node frequent subgraph, if requested
        let mut single_vertex_graph_map: BTreeMap<
            usize,
            BTreeMap<String, (FxHashSet<String>, usize)>> = BTreeMap::new();
        let mut single_vertex_label_frequent_map: BTreeMap<String, usize> = BTreeMap::new(); // 一个 graph 内重复的 vertex 的频繁度只记录一次
        if self.max_pat_min <= 1 {
            self.find_frequent_single_vertex(&mut single_vertex_graph_map, &mut single_vertex_label_frequent_map);
        }

        // 2. Report the single vertex subgraphs
        let mut next_gid: usize = 0;
        
        self.print_frequent_single_vertex(&mut single_vertex_graph_map, &mut single_vertex_label_frequent_map,
            &mut next_gid, process);

        // 3. Subgraphs > Vertices
        // root: [from_label][e_label][to_label] -> Projected
        let mut root: BTreeMap<String, BTreeMap<String, BTreeMap<String, Projected>>> = BTreeMap::new();
        for g in &self.trans {
            for from in &g.vertices {
                let edges: Vec<&Edge> = get_forward_edges(&g, from);
                if edges.is_empty() {
                    continue;
                }
                for edge in &edges {
                    let key_1 = from.label.clone();
                    let root_1 = root.entry(key_1).or_insert(BTreeMap::new());
                    let key_2 = edge.e_label.clone();
                    let root_2 = root_1.entry(key_2).or_insert(BTreeMap::new());
                    let key_3 = g.find_vertex(&edge.to).unwrap().label.clone();
                    let root_3 = root_2.entry(key_3).or_insert(Projected::new());
                    root_3.push(g.id, edge, None);
                }
            }
        }
        let mut dfs_code = DFSCode::new();
        for (from_label_key, from_label_value) in root.iter() {
            for (e_label_key, e_label_value) in from_label_value.iter() {
                for (to_label_key, to_label_value) in e_label_value.iter() {
                    dfs_code.push(
                        0,
                        1,
                        from_label_key.to_string(),
                        e_label_key.to_string(),
                        to_label_key.to_string(),
                    );
                    self.sub_mining(to_label_value, &mut dfs_code, &mut next_gid, process);
                    dfs_code.pop_with_set_result(to_label_value);
                }
            }
        }
        next_gid
    }

    fn find_frequent_single_vertex(&self, single_vertex_graph_map: &mut BTreeMap<
        usize, BTreeMap<String, (FxHashSet<String>, usize)>>,
        single_vertex_label_frequent_map: &mut BTreeMap<String, usize>,
    ) {
        for graph in &self.trans {
            for vertex in &graph.vertices {
                let key = &vertex.label;
                let d = single_vertex_graph_map.entry(graph.id).or_default();
                if d.get(key).is_none() {
                    single_vertex_label_frequent_map
                        .entry(key.to_string())
                        .and_modify(|f| *f += 1)
                        .or_insert(1);
                }
                d.entry(key.to_string())
                    .and_modify(|v| {
                        v.0.insert(vertex.name.clone());
                        v.1 += 1;
                    })
                    .or_insert(([vertex.name.clone()].iter().cloned().collect(), 1));
            }
        }
    }

    fn print_frequent_single_vertex<W: Write + Send + Sync + 'static>(&self, single_vertex_graph_map: &BTreeMap<
            usize, BTreeMap<String, (FxHashSet<String>, usize)>,
        >, single_vertex_label_frequent_map: &BTreeMap<String, usize>,
        next_gid: &mut usize, process: &mut Option<W>,
    ) {
        for (frequent_label, sup) in single_vertex_label_frequent_map.iter() {
            // 判断图之间的支持度
            if sup < &self.min_sup {
                continue;
            }

            let mapped: Vec<(FxHashSet<String>, usize)> = single_vertex_graph_map
                .iter()
                .map(|entry| {
                    entry.1.get(frequent_label).unwrap_or(&(FxHashSet::default(), 0)).clone()
                }).collect();

            // 计算图内部最小、最大支持度
            let mut min = usize::MAX;
            let mut max = usize::MIN;
            for (_, v) in mapped.iter() {
                min = if min > *v { *v } else { min };
                max = if max < *v { *v } else { max };
            }

            if max < self.inner_min_sup {
                continue;
            }

            if let Some(process) = process {
                let gid = next_gid.clone();
                
                let mut g = Graph::new(gid, self.directed);
                g.insert_vertex("result_0", frequent_label);

                let result = mapped.iter().map(|v| v.clone()).reduce(|mut acc, cur| {
                    acc.0.extend(cur.0); // 扩展集合
                    acc.1 += cur.1; // 累加计数
                    acc
                });

                if let Some((set, count)) = result {
                    self.report_single(process, &mut g, *sup, min, max, count, set);
                } else {
                    println!("The map is empty or no matching labels.")
                }
            }
            *next_gid += 1;
        }
    }

    fn sub_mining<W: Write + Send + Sync + 'static>(&self, projected: &Projected, dfs_code: &mut DFSCode,
        next_gid: &mut usize, process: &mut Option<W>,
    ) {
        if self.should_stop_mining(projected, dfs_code, next_gid, process) {
            return;
        }

        /*
         * We just outputted a frequent sub-graph. As it is frequent enough, so
         * mingt be its (n+1)-extension-graphs, hence we enumerate them all.
         */
        let min_rm_path = dfs_code.build_rm_path();
        let min_label = dfs_code.get_dfs(0).from_label.clone();
        let max_to_code = dfs_code.get_dfs(*min_rm_path.get(0).unwrap()).to.clone();

        let (new_fwd_root, new_bck_root) = self.generate_next_root(projected, dfs_code,
            &min_rm_path, &min_label, max_to_code);

        // Test all extended substructures..
        // .. backward
        for (to_key, to_value) in new_bck_root.iter() {
            for (e_label_key, e_label_value) in to_value.iter() {
                dfs_code.push(max_to_code, *to_key, Vertex::NIL_V_LABEL.to_string(),
                    e_label_key.to_string(), Vertex::NIL_V_LABEL.to_string());
                self.sub_mining(e_label_value, dfs_code, next_gid, process);
                dfs_code.pop_with_set_result(e_label_value);
            }
        }
        // .. forward
        for (from_key, from_value) in new_fwd_root.iter().rev() {
            for (e_label_key, e_label_value) in from_value.iter() {
                for (to_label_key, to_label_value) in e_label_value.iter() {
                    dfs_code.push(*from_key, max_to_code + 1, Vertex::NIL_V_LABEL.to_string(),
                        e_label_key.to_string(), to_label_key.to_string());
                    self.sub_mining(to_label_value, dfs_code, next_gid, process);
                    dfs_code.pop_with_set_result(to_label_value);

                }
            }
        }
    }

    fn generate_next_root<'a>(&'a self, projected: &'a Projected<'a>, dfs_code: &DFSCode,
        min_rm_path: &Vec<usize>, min_label: &str, max_to_code: usize,
    ) -> (BTreeMap<usize, BTreeMap<String, BTreeMap<String, Projected<'a>>>>,
        BTreeMap<usize, BTreeMap<String, Projected<'a>>>) {
        
        // [from][e_label][to_label] -> Projected
        let mut new_fwd_root: BTreeMap<usize, BTreeMap<String, BTreeMap<String, Projected>>> = BTreeMap::new();
        // [to][e_label] -> Projected
        let mut new_bck_root: BTreeMap<usize, BTreeMap<String, Projected>> = BTreeMap::new();

        // Enumerate all possible one edge extensions of the current substructure.
        for a_projected in projected.projections.iter() {
            let gid = a_projected.gid;
            let history = History::build(a_projected);

            // backward
            for i in (0..min_rm_path.len()).rev() {
                let e = get_backward(self.trans.get(gid).unwrap(),
                    history.histories.get(*min_rm_path.get(i).unwrap()).unwrap(),
                    history.histories.get(*min_rm_path.get(0).unwrap()).unwrap(), &history);
                if let Some(e) = e {
                    let key_1 = dfs_code.get_dfs(*min_rm_path.get(i).unwrap()).from;
                    let root_1 = new_bck_root.entry(key_1).or_insert(BTreeMap::new());
                    let key_2: &String = &e.e_label;
                    let root_2 = root_1.entry(key_2.to_string()).or_insert(Projected::new());
                    root_2.push(gid, e, Some(&a_projected));
                }
            }
            // pure forward
            let edges: Vec<&Edge> = get_forward_pure(self.trans.get(gid).unwrap(),
                history.histories.get(*min_rm_path.get(0).unwrap()).unwrap(), &min_label, &history);
            if !edges.is_empty() {
                for it in &edges {
                    let root_1 = new_fwd_root.entry(max_to_code).or_insert(BTreeMap::new());
                    let key_2: &String = &it.e_label;
                    let root_2 = root_1.entry(key_2.to_string()).or_insert(BTreeMap::new());
                    let key_3: &String = &self.trans.get(gid).unwrap().find_vertex(&it.to).unwrap().label;
                    let root_3 = root_2.entry(key_3.to_string()).or_insert(Projected::new());
                    root_3.push(gid, it, Some(&a_projected));
                }
            }
            // backtracked forward
            for a_rm_path in min_rm_path {
                let edges: Vec<&Edge> = get_forward_rm_path(self.trans.get(gid).unwrap(),
                history.histories.get(*a_rm_path).unwrap(), &min_label, &history);
                if edges.is_empty() {
                    continue;
                }
                for it in &edges {
                    let key_1 = dfs_code.get_dfs(*a_rm_path).from;
                    let root_1 = new_fwd_root.entry(key_1).or_insert(BTreeMap::new());
                    let key_2: &String = &it.e_label;
                    let root_2 = root_1.entry(key_2.to_string()).or_insert(BTreeMap::new());
                    let key_3: &String = &self.trans.get(gid).unwrap().find_vertex(&it.to).unwrap().label;
                    let root_3 = root_2.entry(key_3.to_string()).or_insert(Projected::new());
                    root_3.push(gid, it, Some(&a_projected));
                }
            }
        }
        (new_fwd_root, new_bck_root)
    }

    fn should_stop_mining<W: Write + Send + Sync + 'static>(&self, projected: &Projected, dfs_code: &mut DFSCode,
        next_gid: &mut usize, process: &mut Option<W>,
    ) -> bool {
        // Check if the pattern is frequent enough, between graphs
        let sup: usize = support(projected);
        if sup < self.min_sup {
            return true;
        }
        // Check if the pattern is frequent enough, within graphs
        let (min_inner_sup, max_inner_sup) = inner_support(projected);
        if max_inner_sup < self.inner_min_sup {
            return true;
        }
        // Check if the pattern is not min
        if !self.is_min(dfs_code) {
            return true;
        }

        /*
         * In case we have a valid upper bound and our graph already exceeds it,
         * return. Note: we do not check for equality as the DSF exploration may
         * still add edges within an existing sub-graph, without increasing the
         * number of nodes.
         */
        if self.max_pat_max >= self.max_pat_min && dfs_code.count_node() > self.max_pat_max {
            return true;
        }

        // Output the frequent substructures
        if let Some(process) = process {
            let gid = next_gid.clone();
            self.report(
                sup,
                min_inner_sup,
                max_inner_sup,
                projected.projections.len(),
                dfs_code,
                gid,
                process,
            );
        }
        *next_gid += 1;

        false
    }
}

// is_min
impl GSpan {
    // 判断 dfs_code 是否为最小的
    fn is_min(&self, dfs_code: &mut DFSCode) -> bool {
        if dfs_code.dfs_vec.len() == 1 {
            return true;
        }

        let graph_is_min = dfs_code.to_graph(0, self.directed);

        let mut dfs_code_is_min = DFSCode::new();

        // [from_label][e_label][to_label] -> Projected
        // BTreeMap 在 Rust 中会自动根据键进行排序
        let mut root: BTreeMap<String, BTreeMap<String, BTreeMap<String, Projected>>> = BTreeMap::new();

        for from in &graph_is_min.vertices {
            let edges: Vec<&Edge> = get_forward_edges(&graph_is_min, from);
            for it in &edges {
                let key_1 = &it.from_label;
                let root_1 = root.entry(key_1.to_string()).or_insert(BTreeMap::new());
                let key_2 = &it.e_label;
                let root_2 = root_1.entry(key_2.to_string()).or_insert(BTreeMap::new());
                let key_3 = &it.to_label;
                let root_3 = root_2.entry(key_3.to_string()).or_insert(Projected::new());
                // 创建初始化子图：一个 Edge 就是一个最小子图
                root_3.push(graph_is_min.id, it, None);
            }
        }

        // 获取最小 [from_label][e_label][to_label] 的组合
        let from_label_map_entry = root.first_key_value().unwrap();
        let from_label_map_value = from_label_map_entry.1;
        let e_label_map_entry = from_label_map_value.first_key_value().unwrap();
        let e_label_map_value = e_label_map_entry.1;
        let to_label_map_entry = e_label_map_value.first_key_value().unwrap();
        let to_label_map_value = to_label_map_entry.1;
        dfs_code_is_min.push(0, 1, from_label_map_entry.0.to_string(),
            e_label_map_entry.0.to_string(), to_label_map_entry.0.to_string());
        
        self.is_min_dfscode(to_label_map_value, dfs_code, &mut dfs_code_is_min, &graph_is_min)
    }

    /**
     * 根据当前的 DFSCode 序列重构出图
     * 以边序列遍历方式递归构造这幅图的最小 min-DFSCode
     * 每一步递归过程都对 DFSCode 序列合 min-DFSCode 序列进行对比，一旦有某个 DFSCode 不同，那么这个 DFSCode 就不是最小的，可以剪枝
     */
    fn is_min_dfscode(&self, projected: &Projected, dfs_code: &DFSCode, dfs_code_is_min: &mut DFSCode,
        graph_is_min: &Graph,
    ) -> bool {
        let min_rm_path = dfs_code_is_min.build_rm_path();
        let max_to_code = dfs_code_is_min.get_dfs(*min_rm_path.get(0).unwrap()).to.clone();

        {
            // backward 情况下是否最小: [e_label] -> Projected
            let (root, new_to) = self.backward_expand(projected, dfs_code_is_min, graph_is_min, &min_rm_path);

            if let Some(mut root) = root {
                let e_label_map_entry = root.first_entry().unwrap();
                dfs_code_is_min.push(max_to_code, new_to, Vertex::NIL_V_LABEL.to_string(),
                    e_label_map_entry.key().to_string(), Vertex::NIL_V_LABEL.to_string());
                let len = dfs_code_is_min.dfs_vec.len();
                if dfs_code.get_dfs(len - 1).ne(dfs_code_is_min.get_dfs(len - 1)) {
                    return false;
                }
                return self.is_min_dfscode(e_label_map_entry.get(), dfs_code, dfs_code_is_min, graph_is_min);
            }
        }

        {
            // forward 情况下是否最小
            let (root, new_from) = self.forward_expand(projected, dfs_code_is_min, graph_is_min,
                &min_rm_path, &max_to_code);

            if let Some(root) = root {
                let (e_label_map_key, to_label_map_key, to_label_map_value) = self.get_first_entry_of_e_to_p(&root);
                dfs_code_is_min.push(new_from, max_to_code + 1, Vertex::NIL_V_LABEL.to_string(),
                    e_label_map_key.to_string(), to_label_map_key.to_string());
                
                let len: usize = dfs_code_is_min.dfs_vec.len();
                if dfs_code.get_dfs(len - 1).ne(dfs_code_is_min.get_dfs(len - 1)) {
                    return false;
                }
                return self.is_min_dfscode(to_label_map_value, dfs_code, dfs_code_is_min, graph_is_min);
            }
        }
        true
    }

    fn backward_expand<'a>(&self, projected: &'a Projected, dfs_code_is_min: &mut DFSCode,
        graph_is_min: &'a Graph, min_rm_path: &Vec<usize>,
    ) -> (Option<BTreeMap<String, Projected<'a>>>, usize) {
        let mut root: Option<BTreeMap<String, Projected>> = None;
        let mut new_to: usize = 0;
        // 逆序获取 min_rm_path
        // min_rm_path 原本是从大到小的 dfs_vec 索引，逆过来就是从小到大
        for i in (1..min_rm_path.len()).rev() {
            root = self.generate_e_p_map(projected, graph_is_min, &min_rm_path, i);
            if root.is_some() {
                new_to = dfs_code_is_min.get_dfs(*min_rm_path.get(i).unwrap()).from;
                break;
            }
        }
        (root, new_to)
    }

    fn forward_expand<'a>(&self, projected: &'a Projected, dfs_code_is_min: &mut DFSCode,
        graph_is_min: &'a Graph, min_rm_path: &Vec<usize>, max_to_code: &usize,
    ) -> (Option<BTreeMap<String, BTreeMap<String, Projected<'a>>>>, usize) {
        let min_label = dfs_code_is_min.get_dfs(0).from_label.clone();

        let mut new_from: usize = 0;
        let mut root: Option<BTreeMap<String, BTreeMap<String, Projected>>> =
            self.generate_e_to_p_map(projected, graph_is_min, |history| {
                let last_rm_path_edge = history.histories.get(*min_rm_path.get(0).unwrap()).unwrap();
                get_forward_pure(graph_is_min, last_rm_path_edge, &min_label, &history)
            });
        
        if root.is_some() {
            new_from = max_to_code.clone();
        } else {
            // min_rm_path 是从大到小的 dfs_vec 索引
            for i in 0..min_rm_path.len() {
                root = self.generate_e_to_p_map(projected, graph_is_min, |history| {
                    let cur_rm_path_edge = history.histories.get(*min_rm_path.get(i).unwrap()).unwrap();
                    get_forward_rm_path(graph_is_min, cur_rm_path_edge, &min_label, &history)
                });
                if root.is_some() {
                    new_from  = dfs_code_is_min.get_dfs(*min_rm_path.get(i).unwrap()).from;
                    break;
                }
            }
        }
        (root, new_from)
    }

    // 生成 [e_label] -> Projected 结构的 BTreeMap
    fn generate_e_p_map<'a>(
        &self,
        projected: &'a Projected,
        graph_is_min: &'a Graph,
        min_rm_path: &Vec<usize>,
        i: usize,
    ) -> Option<BTreeMap<String, Projected<'a>>> {
        // [e_label] -> Projected
        let mut root: BTreeMap<String, Projected> = BTreeMap::new();
        // 遍历产物 子图
        for cur in projected.projections.iter() {
            let cur = &**cur;
            let history: History = History::build(cur);
            // 获取最尾边重点到当前边起点的反向边
            let backward_edge = get_backward(
                graph_is_min, 
                history.histories.get(*min_rm_path.get(i).unwrap()).unwrap(), // 路径当前边
                history.histories.get(*min_rm_path.get(0).unwrap()).unwrap(), // 路径最尾边
                &history,
            );
            if let Some(backward_edge) = backward_edge {
                let key_1 = backward_edge.e_label.clone();
                let root_1: &mut Projected = root.entry(key_1).or_insert(Projected::new());
                root_1.push(0, backward_edge, Some(cur));
            }
        }
        if root.is_empty() { None } else { Some(root) }
    }

    // 生成 [e_label][to_label] -> Projected 结构的 BTreeMap
    fn generate_e_to_p_map<'a, F>(
        &self,
        projected: &'a Projected,
        graph_is_min: &'a Graph,
        generate_edges: F,
    ) -> Option<BTreeMap<String, BTreeMap<String, Projected<'a>>>>
    where F: Fn(&History) -> Vec<&'a Edge> {
        // [e_label][to_label] -> Projected
        let mut root: BTreeMap<String, BTreeMap<String, Projected>> = BTreeMap::new();

        for cur in projected.projections.iter() {
            let history: History = History::build(cur);
            let edges = generate_edges(&history);
            if !edges.is_empty() {
                for it in edges {
                    let key_1 = it.e_label.clone();
                    let root_1 = root.entry(key_1).or_insert(BTreeMap::new());
                    let key_2 = graph_is_min.vertex_name_label_map.get(&it.to).unwrap();
                    let root_2 = root_1.entry(key_2.to_string()).or_insert(Projected::new());
                    root_2.push(0, it, Some(cur));
                }
            }
        }
        if root.is_empty() { None } else { Some(root) }
    }

    fn get_first_entry_of_e_to_p<'a>(
        &self,
        root: &'a BTreeMap<String, BTreeMap<String, Projected>>,
    ) -> (&'a str, &'a str, &'a Projected<'a>) {
        let e_label_map_entry = root.first_key_value().unwrap();
        let e_label_map_value = e_label_map_entry.1;
        let to_label_map_entry = e_label_map_value.first_key_value().unwrap();
        let to_label_map_value = to_label_map_entry.1;
        (
            e_label_map_entry.0,
            to_label_map_entry.0,
            to_label_map_value,
        )
    }
}

impl GSpan {
    fn report_single<W: Write + Send + Sync + 'static>(
        &self,
        out: &mut W,
        g: &mut Graph,
        sup: usize,
        min_inner_sup: usize,
        max_inner_sup: usize,
        total: usize,
        names: FxHashSet<String>,
    ) {
        if self.max_pat_max >= self.max_pat_min && g.vertices.len() >= self.max_pat_max {
            return;
        }
        if self.max_pat_min > 0 && g.vertices.len() < self.max_pat_min {
            return;
        }
        out.write(
            &*g.to_str_repr(Some((sup, min_inner_sup, max_inner_sup, total))).into_bytes()
        ).unwrap();
        out.write(b"\n").unwrap();
        let vec: Vec<String> = names.into_iter().collect();
        out.write(&vec.join(",").into_bytes()).unwrap();
        out.write(b"\n").unwrap();
    }

    fn report<W: Write + Send + Sync + 'static>(
        &self,
        sup: usize,
        min_inner_sup: usize,
        max_inner_sup: usize,
        total: usize,
        dfs_code: &DFSCode,
        gid: usize,
        out: &mut W,
    ) {
        if self.max_pat_max >= self.max_pat_min && dfs_code.count_node() > self.max_pat_max {
            return;
        }
        if self.max_pat_min > 0 && dfs_code.count_node() < self.max_pat_min {
            return;
        }
        
        let g = dfs_code.to_graph(gid, self.directed);
        out.write(b"-------\n").unwrap();
        out.write(
            &*g.to_str_repr(Some((sup, min_inner_sup, max_inner_sup, total))).into_bytes()
        ).unwrap();
        out.write(b"\n").unwrap();
    }
}

#[cfg(test)]
mod tests {
    use crate::gspan::result::MaxDFSCodeGraphResult;

    use super::*;

    #[test]
    fn test_run_single_graph() {
        // JSON 文件路径
        let filename = r#"json\single-graph.json"#;

        match Graph::graph_from_file(&filename, true) {
            Ok(graph) => {
                println!("{}", graph.to_str_repr(None));
                
                let gspan = GSpan::new_with_out_path(
                    vec![graph],
                    1,
                    2,
                    1,
                    10,
                    true,
                    "out-single.txt",
                    OutType::TXT,
                );

                let subgraphs = gspan.run(&mut Some(BufWriter::new(File::create("out-process-single.txt").unwrap())));

                let singleton = MaxDFSCodeGraphResult::get_instance();
                
                assert_eq!(8, subgraphs);
                assert_eq!(5, singleton.get_value_len());
                assert_eq!(12, singleton.get_sum_subgraphs());
            },
            Err(err) => {
                println!("Error: {}", err);
            }
        }
    }

    #[test]
    fn test_run_lenet_graph() {
        // JSON 文件路径
        let filename = r#"json\lenet.json"#;

        match Graph::graph_from_file(&filename, true) {
            Ok(graph) => {
                println!("{}", graph.to_str_repr(None));
                
                let file = File::create("out-t.txt").unwrap();
                let buffered_writer = BufWriter::new(file);

                let gspan = GSpan::new_with_stream(
                    vec![graph],
                    1,
                    2,
                    1,
                    10,
                    true,
                    buffered_writer,
                    OutType::TXT,
                );

                let subgraphs = gspan.run(&mut Some(BufWriter::new(File::create("out-t-process.txt").unwrap())));

                let singleton = MaxDFSCodeGraphResult::get_instance();
                
                assert_eq!(10, subgraphs);
                assert_eq!(2, singleton.get_value_len());
                assert_eq!(4, singleton.get_sum_subgraphs());
            },
            Err(err) => {
                println!("Error: {}", err);
            }
        }
    }
}