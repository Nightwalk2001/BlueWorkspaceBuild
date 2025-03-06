use prost_build::Config;

fn main() {
    let mut config = Config::new();
    config.type_attribute(".", "#[derive(serde::Serialize, serde::Deserialize)]");
    config
        .compile_protos(&["proto/mindir.proto", "proto/geir.proto", "proto/onnx.proto"], &["proto"])
        .unwrap();
}
