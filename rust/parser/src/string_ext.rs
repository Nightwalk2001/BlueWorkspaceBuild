use std::fmt::Write;

use smartstring::alias::String;

use crate::StdString;

/// This trait is designed to simplify code and enhance readability.
pub trait SmartStringExt {
    fn null() -> String {
        String::from("NULL")
    }
    fn from_slice(origin: &[StdString]) -> Vec<String>;

    fn from_bool(value: bool) -> String {
        match value {
            true => "true",
            false => "false",
        }
        .into()
    }

    fn from_f32(value: f32) -> String;

    fn from_f64(value: f64) -> String;

    fn from_i32(value: i32) -> String;

    fn from_i64(value: i64) -> String;

    fn from_vecu8(value: &[u8]) -> String;

    fn from_bools(value: &[bool]) -> String {
        let mut ret = String::new();
        let mut iter = value.iter().peekable();

        while let Some(&v) = iter.next() {
            ret.push_str(&String::from_bool(v));
            iter.peek().is_some().then(|| ret.push(','));
        }

        ret
    }

    fn from_f32s(value: &[f32]) -> String;

    fn from_f64s(value: &[f64]) -> String;

    fn from_i64s(value: &[i64]) -> String;

    fn from_2dvecu8(value: &Vec<Vec<u8>>) -> String;

    fn vecu8s2strings(value: &[Vec<u8>]) -> Vec<String> {
        value.iter().map(|x| String::from_vecu8(x)).collect()
    }
}

/// This macro is a boilerplate code, because slice to
/// string conversion is actually an iterative call to the conversion method.
/// The following is a sample code of macro expansion.
///
/// ## Expanded
/// ```
/// use parser::SmartStringExt;
/// use smartstring::alias::String;
///
/// fn from_i64s(value: &[i64]) -> String {
///     let mut ret = String::new();
///
///     let mut iter = value.iter().peekable();
///     while let Some(&v) = iter.next() {
///         iter.peek().is_some().then(|| ret.push_str(", "));
///         let _ = write!(&mut ret, "{}", String::from_i64(v));
///     }
///
///     ret
/// }
/// ```
/// ## Note
/// In order to avoid calling the join method, a for loop is used here instead
/// of slice iteration, but this problem has not been avoided:
/// [`String::from_i64`] returns a String type. Besides,
/// we can see that this String is converted from &str, while the parameter of
/// [`String::push_str`] is of &str type, that is,
/// there is a waste of left-hand-right-hand operation here.
macro_rules! impl_from_slice {
    ($name:ident, $ty:ty, $method:ident) => {
        fn $name(value: &[$ty]) -> String {
            let mut ret = String::new();

            let mut iter = value.iter().peekable();
            while let Some(&v) = iter.next() {
                if !ret.is_empty() {
                    ret.push_str(", ");
                }
                let _ = write!(&mut ret, "{}", String::$method(v));
            }

            ret
        }
    };
}

/// ## Performance
///
/// This implementation uses the [`ryu`] and [`itoa`] crates for converting numbers to strings.
/// These crates generally offer better performance compared to the standard library implementations.
///
/// ## Note
/// The code duplication here is unavoidable because [`rustc`] needs to know the size of a type at compile time.
/// Therefore, a unified `Float` type cannot be used, as `f32` (4 bytes) and `f64` (8 bytes) have different sizes.
impl SmartStringExt for String {
    fn from_slice(origin: &[StdString]) -> Vec<String> {
        origin.iter().map(|i| i.into()).collect()
    }

    fn from_f32(value: f32) -> String {
        let mut buf = ryu::Buffer::new();
        let s = buf.format(value);
        String::from(s)
    }

    fn from_f64(value: f64) -> String {
        let mut buf = ryu::Buffer::new();
        let s = buf.format(value);
        String::from(s)
    }

    fn from_i32(value: i32) -> String {
        let mut buf = itoa::Buffer::new();
        let s = buf.format(value);
        String::from(s)
    }

    fn from_i64(value: i64) -> String {
        let mut buf = itoa::Buffer::new();
        let s = buf.format(value);
        String::from(s)
    }

    /// Performance
    /// Using [`from_utf8_unchecked`] can provide higher performance,
    /// unless the file is modified or an error occurs when writing
    /// the file, there will be no illegal strings.
    fn from_vecu8(value: &[u8]) -> String {
        match std::str::from_utf8(value) {
            Ok(s) => String::from(s),
            Err(_) => String::from("InvalidUTF8Str"),
        }
    }

    impl_from_slice!(from_f32s, f32, from_f32);
    impl_from_slice!(from_f64s, f64, from_f64);
    impl_from_slice!(from_i64s, i64, from_i64);

    fn from_2dvecu8(value: &Vec<Vec<u8>>) -> String {
        let mut ret = String::new();
        let mut iter = value.iter().peekable();

        while let Some(v) = iter.next() {
            ret.push_str(&String::from_vecu8(v));
            iter.peek().is_some().then(|| ret.push(';'));
        }

        ret
    }
}
