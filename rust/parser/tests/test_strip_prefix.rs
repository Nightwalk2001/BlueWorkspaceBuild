use std::borrow::Cow;

use parser::StrExt;
use smartstring::alias::String as SmartString;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_str_strip_prefix() {
        let s = "hello world";
        assert_eq!(s.try_strip_prefix("hello "), "world");
        assert_eq!(s.try_strip_prefix("hello"), " world");
        assert_eq!(s.try_strip_prefix("world"), s);
    }

    #[test]
    fn test_smartstring_strip_prefix() {
        let s = SmartString::from("hello world");
        assert_eq!(s.try_strip_prefix("hello "), "world");
        assert_eq!(s.try_strip_prefix("hello"), " world");
        assert_eq!(s.try_strip_prefix("world"), s.as_str());
    }

    #[test]
    fn test_option_smartstring_strip_prefix_some() {
        let s = Some(SmartString::from("hello world"));
        assert_eq!(s.try_strip_prefix("hello "), "world");
        assert_eq!(s.try_strip_prefix("hello"), " world");
        assert_eq!(s.try_strip_prefix("world"), s.as_ref().unwrap());
    }

    #[test]
    fn test_option_smartstring_strip_prefix_none() {
        let s: Option<SmartString> = None;
        assert_eq!(s.try_strip_prefix("hello"), "");
    }

    #[test]
    fn test_string_strip_prefix() {
        let s = String::from("hello world");
        assert_eq!(s.try_strip_prefix("hello "), "world");
        assert_eq!(s.try_strip_prefix("hello"), " world");
        assert_eq!(s.try_strip_prefix("world"), s.as_str());
    }

    #[test]
    fn test_option_string_strip_prefix_some() {
        let s = Some(String::from("hello world"));
        assert_eq!(s.try_strip_prefix("hello "), "world");
        assert_eq!(s.try_strip_prefix("hello"), " world");
        assert_eq!(s.try_strip_prefix("world"), s.as_ref().unwrap());
    }

    #[test]
    fn test_option_string_strip_prefix_none() {
        let s: Option<String> = None;
        assert_eq!(s.try_strip_prefix("hello"), "");
    }

    #[test]
    fn test_cow_strip_prefix() {
        let s = Cow::from("hello world");
        assert_eq!(s.try_strip_prefix("hello "), "world");
        assert_eq!(s.try_strip_prefix("hello"), " world");
        assert_eq!(s.try_strip_prefix("world"), s.as_ref());
    }

    #[test]
    fn test_option_cow_strip_prefix_some() {
        let s = Some(Cow::from("hello world"));
        assert_eq!(s.try_strip_prefix("hello "), "world");
        assert_eq!(s.try_strip_prefix("hello"), " world");
        assert_eq!(s.try_strip_prefix("world"), s.as_ref().unwrap());
    }

    #[test]
    fn test_option_cow_strip_prefix_none() {
        let s: Option<Cow<'static, str>> = None;
        assert_eq!(s.try_strip_prefix("hello"), "");
    }
}
