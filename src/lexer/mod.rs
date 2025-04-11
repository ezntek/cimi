use std::fmt::{self, Display};


pub enum Keyword {
    // 
}

pub enum Operator {
    // < > <= >= == != 
    // + - / * ^ %
    Gt,
    Lt,
    Geq,
    Leq,
    Eq,
    Neq,
    Add,
    Sub,
    Div,
    Mul,
    Pow,
    Mod,
    // Functional Operators
    Assign,
}

impl Display for Operator {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use Operator as O;
        
        let s = match &self {
            O::Gt => ">",
            O::Lt => "<",
            O::Geq => ">=",
            O::Leq => "<=",
            O::Eq => "==",
            O::Neq => "!=",
            O::Add => "+",
            O::Sub => "-",
            O::Div => "/",
            O::Mul => "*",
            O::Pow => "^",
            O::Mod => "%",
            O::Assign => "=",
        };
        
        write!(f, "{s}")
    } 
}


pub enum Separator {

}

pub enum Type {

}

pub enum TokenType {
    Keyword(Keyword),
    Ident(String),
    Literal(String),
    Operator(Operator),
    Separator(Separator),
    Type(Type),
}
