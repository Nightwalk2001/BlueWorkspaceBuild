use std::{
    fs::File,
    io::{BufReader, Read, Result},
};

use prost::Message;
use smartstring::alias::String;

pub mod geir;
pub use geir::*;

pub mod mindir;
pub use mindir::*;

pub mod onnx;
pub use onnx::*;

use crate::{AttrValue, AttrValue::*};

/// ## Note
/// This generic function is designed for parsing proto buffer files,
/// using bytes parsing, which is very high performance.
///
/// We may never not support proto buffer text files.
/// For comparison:
/// - `30-line` [`pbtxt`] file (using [`protobuf`]) takes `1.4ms`,
/// - `1800-line` [`pb`] file (using [`prost`]) takes `480μs`.
///
/// Instead, we recommend using the provided [`python`] script or writing one yourself,
/// which only takes several lines to convert the pbtxt format to pb format
///
/// ```py
/// message = ModelDef()
/// with open(pbtxt_file_path, 'r') as f:
///     text_format.Merge(f.read(), message)
/// with open(binary_file_path, 'wb') as f:
///     f.write(message.SerializeToString())
/// ```
///
/// ## Performance Improvement
/// If you parse binary directly without using prost,
/// which can reduce memory allocation and loop process
fn parse_pb<T>(path: &str) -> Result<T>
where
    T: Default + Message,
{
    let file = File::open(path)?;
    let mut reader = BufReader::new(file);
    let mut buffer = vec![];
    reader.read_to_end(&mut buffer)?;

    let model: T = Message::decode(&*buffer)?;

    Ok(model)
}

/// ## Explanation
/// `Tensor` usually has two attributes: [`dtype`] and [`dims`],
/// and sometimes also has attributes such as [`device`] and [`layout`].
/// If this structure is sent directly to the front-end for processing,
/// most of the time, the front-end only do a string concatenation,
/// which brings the following disadvantages:
/// - Increased communication data volume
/// - Deeper object hierarchy (which may cause a sharp increase in JS GC time)
/// - Slower string concatenation that must be run [`every-time`] rendering
///
/// ## Note
/// However, it cannot be ignored that if the edge width needs to be adjusted
/// according to the data dimension, the design needs to be reconsidered.
trait TensorFormatter {
    fn fmt(&self) -> String;
}

#[inline]
fn format_tensors<T>(tensors: &[T]) -> AttrValue
where
    T: TensorFormatter,
{
    if tensors.len() < 2 {
        return StringLike(tensors[0].fmt());
    }
    TensorVals(tensors.iter().map(|t| t.fmt()).collect())
}
