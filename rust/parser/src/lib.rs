#![allow(unused_doc_comments)]

pub mod model;
pub use model::*;

pub mod processors;
pub use processors::*;

pub mod str_ext;
pub use str_ext::StrExt;

pub mod string_ext;
pub use string_ext::SmartStringExt;

pub type StdString = String;
