use subgraph::subgraphs_onnx;

fn main() {
    let path = r#"C:\Users\nightwalk\Downloads\squeezenet1.geonnx"#;
    match subgraphs_onnx(path, 2, 10) {
        Some(subgraphs) => println!("{subgraphs:?}"),
        None => println!("Error")
    }
}
