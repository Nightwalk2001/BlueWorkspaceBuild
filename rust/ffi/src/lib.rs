mod geometry;
mod parse_layout;

use std::ffi::{c_char, CStr, CString};

use parse_layout::*;
use serde::Deserialize;
use subgraph::{subgraphs_geir, subgraphs_mindir, subgraphs_onnx};

use self::FileType::*;

enum FileType {
    ONNX,
    MindIR,
    GeIR,
    Unsupported,
}

impl From<&str> for FileType {
    fn from(value: &str) -> Self {
        if value.ends_with(".onnx") {
            return ONNX;
        } else if value.ends_with(".mindir") {
            return MindIR;
        } else if value.ends_with(".geir") {
            return GeIR;
        }
        Unsupported
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn layout_rs(path: *const c_char) -> *mut c_char {
    let path = unsafe {
        assert!(!path.is_null());
        CStr::from_ptr(path).to_str().unwrap()
    };

    let ret = match FileType::from(path) {
        ONNX => layout_onnx(path),
        MindIR => layout_mindir(path),
        GeIR => layout_geir(path),
        Unsupported => unimplemented!(),
    };

    let json_string = serde_json::to_string(&ret).unwrap();

    let c_string = CString::new(json_string).unwrap();

    c_string.into_raw()
}

#[derive(Deserialize)]
struct SubgraphParams {
    path: String,
    min: usize,
    max: usize,
}

#[unsafe(no_mangle)]
pub extern "C" fn subgraph_rs(data: *const c_char) -> *mut c_char {
    let data = unsafe {
        assert!(!data.is_null());
        CStr::from_ptr(data).to_str().unwrap()
    };

    let SubgraphParams { path, min, max } = serde_json::from_str::<SubgraphParams>(data).unwrap();
    let path = path.as_str();

    let ret = match FileType::from(path) { 
        ONNX => subgraphs_onnx(path, min, max),
        MindIR => subgraphs_mindir(path, min, max),
        GeIR => subgraphs_geir(path, min, max),
        Unsupported => unimplemented!()
    };

    let json_string = serde_json::to_string(&ret).unwrap();

    let c_string = CString::new(json_string).unwrap();

    c_string.into_raw()
}

#[unsafe(no_mangle)]
pub extern "C" fn free_string_rs(s: *mut c_char) {
    if s.is_null() {
        return;
    }

    unsafe { drop(CString::from_raw(s)); }
}
