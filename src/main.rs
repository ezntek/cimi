use std::{fs::File, io::{BufReader, Read}};

use clap::Parser;
use colored::Colorize;

#[derive(Parser)]
#[command(version, about, long_about = None)]
struct Args {
    file: String
}

const VERSION: &str = "0.1.0";

fn main() {
    println!("{} version {VERSION}", "CimiLang Frontend".bold());

    let args = Args::parse();
    
    let fp = File::open(&args.file).expect("failed to open file");
    let mut br = BufReader::new(fp);

    let mut buf = String::new();
    br.read_to_string(&mut buf).expect("failed to read file");

    println!("{}", buf);
}
