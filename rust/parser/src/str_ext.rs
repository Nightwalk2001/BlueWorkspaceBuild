use std::borrow::Cow;

use smartstring::alias::String as SmartString;

pub trait StrExt {
    fn try_strip_prefix(&self, prefix: &str) -> &str;

    #[inline]
    fn strip_prefix1(&self, prefix: &str) -> SmartString {
        self.try_strip_prefix(prefix).into()
    }
}

impl StrExt for &str {
    #[inline]
    fn try_strip_prefix(&self, prefix: &str) -> &str {
        match self.strip_prefix(prefix) {
            Some(ps) => ps,
            _ => self,
        }
    }
}

impl StrExt for SmartString {
    #[inline]
    fn try_strip_prefix(&self, prefix: &str) -> &str {
        match self.as_str().strip_prefix(prefix) {
            Some(ps) => ps,
            _ => self,
        }
    }
}

impl StrExt for Option<SmartString> {
    #[inline]
    fn try_strip_prefix(&self, prefix: &str) -> &str {
        match self {
            Some(s) => s.try_strip_prefix(prefix),
            _ => "",
        }
    }
}

impl StrExt for String {
    #[inline]
    fn try_strip_prefix(&self, prefix: &str) -> &str {
        match self.as_str().strip_prefix(prefix) {
            Some(ps) => ps,
            _ => self,
        }
    }
}

impl StrExt for Option<String> {
    #[inline]
    fn try_strip_prefix(&self, prefix: &str) -> &str {
        match self {
            Some(s) => s.try_strip_prefix(prefix),
            _ => "",
        }
    }
}

impl<'a> StrExt for Cow<'a, str> {
    #[inline]
    fn try_strip_prefix(&self, prefix: &str) -> &str {
        match self.strip_prefix(prefix) {
            Some(s) => s,
            _ => self,
        }
    }
}

impl<'a> StrExt for Option<Cow<'a, str>> {
    #[inline]
    fn try_strip_prefix(&self, prefix: &str) -> &str {
        match self {
            Some(s) => s.try_strip_prefix(prefix),
            None => "",
        }
    }
}
