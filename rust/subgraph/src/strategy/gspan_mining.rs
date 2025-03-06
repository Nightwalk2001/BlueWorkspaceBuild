use std::{
    fs::{File, OpenOptions}, io::{BufWriter, Read, Seek, Write}, sync::mpsc::{self, Receiver, Sender}, thread, time::Instant
};

use crate::{gspan::{
    gspan::GSpan,
    models::graph::Graph,
    result::{JSONResult, MaxDFSCodeGraphResult, OutType},
}, strategy::config::{InputSource, OutputPath, ProcessPath}};

use super::mining_strategy::MiningStrategy;

pub struct GSpanMining;

impl MiningStrategy for GSpanMining {
    fn run(&self, args: super::Config) -> Vec<JSONResult> {
        let graphs = match args.get_input_source() {
            InputSource::File(input_file) => {
                let graph = Graph::graph_from_file(&input_file, false);
                match graph {
                    Ok(graph) => vec![graph],
                    Err(err) => panic!("{}", err.to_string()),
                }
            }
            InputSource::Graphs(graphs) => graphs.to_vec(),
        };

        let gspan = match args.get_output_path() {
            OutputPath::File(file) => GSpan::new_with_out_path(graphs, args.get_min_support(),
                args.get_min_inner_support(), args.get_min_vertices(), args.get_max_vertices(), true, file,
                args.get_output_type().clone()
            ),
            OutputPath::None => GSpan::new(graphs, args.get_min_support(), args.get_min_inner_support(),
                args.get_min_vertices(), args.get_max_vertices(), true, args.get_output_type().clone(),
            ),
        };

        let mut process_writer: Option<BufWriter<File>> = match args.get_process_path() {
            ProcessPath::None => None,
            ProcessPath::File(file) => Some(BufWriter::new(File::create(file).unwrap())),
        };

        let subgraphs = gspan.run(&mut process_writer);
        println!("Found {} subgraphs", subgraphs);
        let singleton = MaxDFSCodeGraphResult::get_instance();
        println!(
            "Found {}/{} subgraphs (Only Max)",
            singleton.get_value_len(),
            singleton.get_sum_subgraphs()
        );

        fix_json_file(args.get_output_path(), args.get_output_type());
        singleton.get_result()
    }

    fn run_channel(&self, args: super::Config) -> Receiver<String> {
        let now = Instant::now();
        let graphs = match args.get_input_source() {
            InputSource::File(file) => {
                let graph = Graph::graph_from_file(&file, false);
                match graph {
                    Ok(graph) => vec![graph],
                    Err(err) => panic!("{}", err.to_string()),
                }
            }
            InputSource::Graphs(graphs) => graphs.to_vec(),
        };
        for graph in graphs.iter() {
            println!("All good parsing input file. vertex: {}, edge: {}.", graph.vertices.len(), graph.edge_size);
        }
        let alpha = now.elapsed().as_millis();
        println!("Took {}ms", alpha);

        println!("Mining subgraphs..");
        let gspan = match args.get_output_path() {
            OutputPath::File(file) => GSpan::new_with_out_path(graphs, args.get_min_support(),
                args.get_min_inner_support(), args.get_min_vertices(), args.get_max_vertices(), true, file,
                args.get_output_type().clone(),
            ),
            OutputPath::None => GSpan::new(graphs, args.get_min_support(), args.get_min_inner_support(),
                args.get_min_vertices(), args.get_max_vertices(), true, args.get_output_type().clone(),
            ),
        };
        let (tx, rx): (Sender<String>, Receiver<String>) = mpsc::channel();

        let process_path = (*args.get_process_path()).clone();
        thread::spawn(move || {
            let singleton = MaxDFSCodeGraphResult::get_instance();
            singleton.set_channel(true, Some(tx));

            let mut process_writer: Option<BufWriter<File>> = match process_path {
                ProcessPath::None => None,
                ProcessPath::File(file) => Some(BufWriter::new(File::create(file).unwrap())),
            };

            let subgraphs = gspan.run(&mut process_writer);
            let delta = now.elapsed().as_millis();
            println!("Finished.");
            println!("Found {} subgraphs", subgraphs);
            println!(
                "Found {}/{} subgraphs (Only Max)",
                singleton.get_value_len(),
                singleton.get_sum_subgraphs()
            );
            println!("Took {}ms", delta - alpha);
            println!("Total Took {}ms", delta);
            singleton.drop_sender();
        });

        fix_json_file(args.get_output_path(), args.get_output_type());
        rx
    }
}

fn fix_json_file(output_path: &OutputPath, output_type: &OutType) {
    match &output_type {
        OutType::JSON => {
            if let OutputPath::File(filename) = &output_path {
                // 打开文件并读取内容
                let mut file = OpenOptions::new()
                    .read(true)
                    .write(true)
                    .open(filename)
                    .unwrap();

                let mut contents = String::new();
                file.read_to_string(&mut contents).unwrap();

                // 修改内容
                contents.pop();
                contents.pop();
                let new_contents = format!("[{}]", contents);

                // 清空文件并重置文件位置
                file.set_len(0).unwrap();
                file.seek(std::io::SeekFrom::Start(0)).unwrap();

                file.write_all(new_contents.as_bytes()).unwrap();
            }
        }
        _ => {}
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::gspan::result::{MaxDFSCodeGraphResult, OutType};
    use crate::strategy::Config;

    #[test]
    fn test_run_lenet_graph() {
        // JSON 文件路径
        let filename = r#"json\lenet.json"#;

        let gspan_mining = GSpanMining;

        match Config::new(
            filename,
            Some("out-t-process.txt"),
            Some("out-t.txt"),
            OutType::TXT,
            1,
            2,
            1,
            10,
        ) {
            Ok(config) => {
                let result = gspan_mining.run(config);

                let singleton = MaxDFSCodeGraphResult::get_instance();

                assert_eq!(2, singleton.get_value_len());
                assert_eq!(4, singleton.get_sum_subgraphs());
                println!("{:?}", result);
            }
            Err(e) => eprintln!("Failed to create config: {:?}", e),
        }
    }

    #[test]
    fn test_run_lenet_graph_parsed() {
        // JSON 文件路径
        let graph = Graph::graph_from_file(r#"json\lenet.json"#, true).unwrap();

        let gspan_mining = GSpanMining;

        match Config::new_from_graphs(
            vec![graph],
            None,
            Some("out-t.json"),
            OutType::JSON,
            1,
            2,
            1,
            10,
        ) {
            Ok(config) => {
                let result = gspan_mining.run(config);

                let singleton = MaxDFSCodeGraphResult::get_instance();

                assert_eq!(2, singleton.get_value_len());
                assert_eq!(4, singleton.get_sum_subgraphs());
                println!("{:?}", result);
            }
            Err(e) => eprintln!("Failed to create config: {:?}", e),
        }
    }

    #[test]
    fn test_run_channel_lenet_graph() {
        // JSON 文件路径
        let filename = r#"json\lenet.json"#;

        let gspan_mining = GSpanMining;

        match Config::new(
            filename,
            None,
            None,
            OutType::TXT,
            1,
            2,
            1,
            10,
        ) {
            Ok(config) => {
                let rx = gspan_mining.run_channel(config);

                // 在主线程中接收并处理 JSONResult 结果
                for received in rx {
                    println!("Received JSONResult: {:?}", received);
                }
            }
            Err(e) => eprintln!("Failed to create config: {:?}", e),
        };
    }
}