/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
use std::{io::Write, sync::mpsc::Sender};

use rustc_hash::FxHashSet;
use serde::{Deserialize, Serialize};

use crate::{
    gspan::{
        misc::{inner_support, support},
        models::{dfs_code::DFSCode, edge::Edge, projected::Projected},
    },
    io::output::{Edge as OutputEdge, Instance, NodeId, Structure, Vertex as OutputVertex},
};

#[derive(Debug, Clone)]
pub enum OutType {
    TXT,
    JSON,
}

pub enum OutSource<W: Write + Send + Sync + 'static> {
    Channel(Sender<String>),
    Stream(W),
    Path(String),
}

#[derive(Debug, Serialize, Deserialize)]
pub struct JSONResult {
    pub between_sup: usize,
    pub inner_min_sup: usize,
    pub inner_max_sup: usize,
    pub total: usize,
    pub structure: Structure,
    pub instances: Vec<Instance>,
}

#[derive(Default)]
pub struct MaxDFSCodeGraphResult {
    out: Option<Box<dyn Write + Send + Sync>>,
    out_type: Option<OutType>,
    channel: bool, // 如果 channel 为 true，则 out 流失效
    sender: Option<Sender<String>>,
    min_sup: usize,
    inner_min_sup: usize,
    max_pat_min: usize, // Minimum number of vertices
    max_pat_max: usize, // Maximum number of vertices
    value: Vec<(DFSCode, Vec<FxHashSet<(usize, String)>>)>,
}

// // 单例结构体
// pub struct MaxDFSCodeGraphResult {
//     inner: Arc<Mutex<MaxDFSCodeGraphResultInner>>,
// }

impl MaxDFSCodeGraphResult {
    // // 提供一个方法来获取唯一的实例
    // pub fn get_instance() -> &'static MaxDFSCodeGraphResult {
    //     static INSTANCE: OnceLock<MaxDFSCodeGraphResult> = OnceLock::new();
    //     INSTANCE.get_or_init(|| {
    //         MaxDFSCodeGraphResult {
    //             inner: Arc::new(Mutex::new(MaxDFSCodeGraphResultInner::default()))
    //         }
    //     })
    // }
    // pub fn new(
    //     min_sup: usize,
    //     inner_min_sup: usize,
    //     max_pat_min: usize, // Minimum number of vertices
    //     max_pat_max: usize, // Maximum number of vertices
    //     out_type: OutType,
    // ) -> Self {
    //     // 参数验证
    //     assert!(max_pat_min <= max_pat_max, "max_pat_min should be less than or equal to max_pat_max");

    //     MaxDFSCodeGraphResult {
    //         min_sup,
    //         inner_min_sup,
    //         max_pat_min,
    //         max_pat_max,
    //         out_type: Some(out_type),
    //         ..MaxDFSCodeGraphResult::default()
    //     }
    // }

    pub fn set_config(
        &mut self,
        min_sup: usize,
        inner_min_sup: usize,
        max_pat_min: usize, // Minimum number of vertices
        max_pat_max: usize, // Maximum number of vertices
        out_type: OutType,
    ) {
        self.min_sup = min_sup;
        self.inner_min_sup = inner_min_sup;
        self.max_pat_min = max_pat_min;
        self.max_pat_max = max_pat_max;
        self.out_type = Some(out_type);
    }

    pub fn add_value(&mut self, dfs_code: &DFSCode, projected: &Projected) -> bool {
        // Check if the pattern is frequent enough, between graphs
        let sup: usize = support(projected);
        if sup < self.min_sup {
            return false;
        }
        // Check if the pattern is frequent enough, inner graph
        let (_min_inner_sup, max_inner_sup) = inner_support(projected);
        if max_inner_sup < self.inner_min_sup {
            return false;
        }
        // Check if the dfs_code vertices.len in [max_pat_min, max_pat_max]
        if self.max_pat_max >= self.max_pat_min && dfs_code.count_node() > self.max_pat_max {
            return false;
        }
        if self.max_pat_min > 0 && dfs_code.count_node() < self.max_pat_min {
            return false;
        }
        let item = (dfs_code.clone(), projected.to_vertex_names_list());
        let edges_list = projected.to_edges_list();
        if self.channel {
            self.send_result(sup, _min_inner_sup, max_inner_sup, &item, edges_list);
        } else if Option::is_some(&self.out) {
            self.write_result(sup, _min_inner_sup, max_inner_sup, &item, edges_list);
        }
        self.value.push(item);
        true
    }

    pub fn get_value_len(&self) -> usize {
        self.value.len()
    }

    pub fn get_result(&self) -> Vec<JSONResult> {
        self.value
            .iter()
            .map(|v| {
                let instances =
                    v.1.iter()
                        .map(|set| {
                            let node_ids = set
                                .iter()
                                .map(|p| NodeId { gid: p.0, nid: p.1.clone() })
                                .collect::<Vec<NodeId>>();
                            return Instance { node_num: node_ids.len(), node_ids, edges: vec![] };
                        })
                        .collect::<Vec<Instance>>();
                return JSONResult {
                    between_sup: 0,
                    inner_min_sup: 0,
                    inner_max_sup: 0,
                    total: instances.len(),
                    structure: Structure { tid: 0, vertices: vec![], edges: vec![] },
                    instances,
                };
            })
            .collect::<Vec<JSONResult>>()
    }

    pub fn get_sum_subgraphs(&self) -> usize {
        self.value.iter().map(|e| e.1.len()).sum()
    }
}

impl MaxDFSCodeGraphResult {
    pub fn set_channel(&mut self, channel: bool, sender: Option<Sender<String>>) {
        if channel {
            self.out = None;
        } else {
            // take 方法会返回 Some(sender)，并把原来的字段设置为 None
            // _sender 离开作用域时会自动调用 drop，因此不需要显式调用 drop
            let _sender = self.sender.take();
        }
        self.sender = sender;
        self.channel = channel;
    }

    pub fn drop_sender(&mut self) {
        if Option::is_some(&self.sender) {
            // take 方法会返回 Some(sender)，并把原来的字段设置为 None
            // _sender 离开作用域时会自动调用 drop，因此不需要显式调用 drop
            let _sender = self.sender.take();
        }
    }

    fn send_result(
        &mut self,
        sup: usize,
        min_inner_sup: usize,
        max_inner_sup: usize,
        item: &(DFSCode, Vec<FxHashSet<(usize, String)>>),
        edges_list: Vec<Vec<&Edge>>,
    ) {
        let id = self.value.len();
        if let Some(out_type) = &self.out_type {
            match out_type {
                OutType::TXT =>
                    if let Some(sender) = &mut self.sender {
                        let line =
                            report_txt(id, sup, min_inner_sup, max_inner_sup, item, edges_list);
                        sender.send(line).unwrap();
                    },
                OutType::JSON =>
                    if let Some(sender) = &mut self.sender {
                        let line =
                            report_json(id, sup, min_inner_sup, max_inner_sup, item, edges_list);
                        sender.send(line).expect("ERR: MaxDFSCodeGraphResult Channel");
                    },
            }
        }
    }
}

impl MaxDFSCodeGraphResult {
    pub fn set_stream<W: Write + Send + Sync + 'static>(&mut self, out: W) {
        self.channel = false;
        self.out = Some(Box::new(out));
    }

    fn write_result(
        &mut self,
        sup: usize,
        min_inner_sup: usize,
        max_inner_sup: usize,
        item: &(DFSCode, Vec<FxHashSet<(usize, String)>>),
        edges_list: Vec<Vec<&Edge>>,
    ) {
        let id = self.value.len();
        if let Some(out_type) = &self.out_type {
            match out_type {
                OutType::TXT => {
                    if let Some(out) = &mut self.out {
                        let line =
                            report_txt(id, sup, min_inner_sup, max_inner_sup, item, edges_list);
                        out.write(&*line.into_bytes()).expect("ERR: MaxDFSCodeGraphResult Stream");

                        // 刷新缓冲区，确保所有数据都被写出
                        if let Err(e) = out.flush() {
                            eprintln!("Failed to flush output after writing lines: {}", e);
                        }
                    }
                }
                OutType::JSON => {
                    if let Some(out) = &mut self.out {
                        let line =
                            report_json(id, sup, min_inner_sup, max_inner_sup, item, edges_list);
                        out.write(&*line.into_bytes()).expect("ERR: MaxDFSCodeGraphResult Stream");
                        out.write(b",\n").expect("ERR: MaxDFSCodeGraphResult Stream");

                        // 刷新缓冲区，确保所有数据都被写出
                        if let Err(e) = out.flush() {
                            eprintln!("Failed to flush output after writing lines: {}", e);
                        }
                    }
                }
            }
        }
    }
}

fn report_txt(
    id: usize,
    sup: usize,
    min_inner_sup: usize,
    max_inner_sup: usize,
    item: &(DFSCode, Vec<FxHashSet<(usize, String)>>),
    edges_list: Vec<Vec<&Edge>>,
) -> String {
    let mut lines: Vec<String> = vec![];

    let g = item.0.to_graph(id, false);
    let total = item.1.len();
    lines.push("\n".to_string());
    lines.push(g.to_str_repr(Some((sup, min_inner_sup, max_inner_sup, total))));
    lines.push("\n".to_string());

    for (index, line) in item.1.iter().enumerate() {
        let vertex_content = line
            .iter()
            .map(|l| format!("{}/{}", l.0, &l.1))
            .collect::<Vec<String>>()
            .join(", ");
        lines.push(format!("${}| {}\n", line.len(), vertex_content));

        let edge_content = edges_list[index]
            .iter()
            .map(|e| {
                format!(
                    " e| {}/{}-{}-{}/{}",
                    &e.from, &e.from_label, &e.e_label, &e.to, &e.to_label
                )
            })
            .collect::<Vec<String>>()
            .join("\n");
        lines.push(edge_content);
        lines.push("\n".to_string());
    }

    lines.join("")
}

fn report_json(
    id: usize,
    sup: usize,
    min_inner_sup: usize,
    max_inner_sup: usize,
    item: &(DFSCode, Vec<FxHashSet<(usize, String)>>),
    edges_list: Vec<Vec<&Edge>>,
) -> String {
    let g = item.0.to_graph(id, false);

    let mut output_vertices: Vec<OutputVertex> = Vec::new();
    let mut output_edges: Vec<OutputEdge> = Vec::new();
    for vertex in &g.vertices {
        output_vertices
            .push(OutputVertex { name: vertex.name.clone(), label: vertex.label.clone() });
        output_edges.extend(vertex.edges.iter().map(|e| OutputEdge {
            from: e.from.clone(),
            to: e.to.clone(),
            from_label: e.from_label.clone(),
            to_label: e.to_label.clone(),
            e_label: e.e_label.clone(),
        }));
    }

    let mut json_result = JSONResult {
        between_sup: sup,
        inner_min_sup: min_inner_sup,
        inner_max_sup: max_inner_sup,
        total: item.1.len(),
        structure: Structure { tid: g.id, vertices: output_vertices, edges: output_edges },
        instances: vec![],
    };

    for (index, line) in item.1.iter().enumerate() {
        let node_ids = line
            .iter()
            .map(|l| NodeId { gid: l.0, nid: l.1.clone() })
            .collect::<Vec<NodeId>>();

        let edges = edges_list[index]
            .iter()
            .map(|e| OutputEdge {
                from: e.from.clone(),
                to: e.to.clone(),
                from_label: e.from_label.clone(),
                to_label: e.to_label.clone(),
                e_label: e.e_label.clone(),
            })
            .collect::<Vec<OutputEdge>>();

        json_result
            .instances
            .push(Instance { node_num: node_ids.len(), node_ids, edges });
    }

    serde_json::to_string(&json_result).expect("Err: Serialization failed")
}

#[cfg(test)]
mod tests {
    use std::{fs::File, io::BufWriter};

    use super::*;

    // 辅助函数用于创建 Edge 并添加到 DFSCode 中
    fn create_and_add_edges(dfs_code: &mut DFSCode) {
        dfs_code.push(1, 2, String::from("a"), String::from("b"), String::from("A"));
        dfs_code.push(2, 3, String::from("b"), String::from("c"), String::from("A"));
        dfs_code.push(2, 1, String::from("b"), String::from("a"), String::from("B"));
    }

    #[test]
    fn test_singleton() {
        // 获取单例实例
        let mut default = MaxDFSCodeGraphResult::default();

        // 创建文件并使用 BufWriter 来提高性能
        let file_path = "out_test.json";
        let file = File::create(file_path).unwrap();
        let buffered_writer = BufWriter::new(file);

        default.set_config(0, 0, 1, 4, OutType::JSON);
        default.set_stream(buffered_writer);

        assert_eq!(0, default.get_value_len());

        let mut dfs_code = DFSCode::new();
        create_and_add_edges(&mut dfs_code);
        let projected = Projected::new();
        default.add_value(&dfs_code, &projected);
        assert_eq!(1, default.get_value_len());
    }
}
