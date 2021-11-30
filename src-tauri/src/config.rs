use std::collections::VecDeque;

use serde::{Deserialize, Serialize};
use tokio::{fs::OpenOptions, io::AsyncWriteExt};

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Config {
    pub saved: VecDeque<String>,
    pub history: VecDeque<String>,
}

pub fn load_config(j: &str) -> anyhow::Result<Config> {
    let c: Config = toml::from_str(&j)?;
    Ok(c)
}

pub async fn write_config(c: &Config) -> anyhow::Result<()> {
    let proj_dirs = directories::ProjectDirs::from("com", "THMonster", "dmlive").unwrap();
    let d = proj_dirs.config_dir();
    let _ = std::fs::create_dir_all(&d);
    let config_path = d.join("revda.toml");
    if !config_path.exists() {
        let _ = tokio::fs::File::create(&config_path);
    }
    let mut f = OpenOptions::new().write(true).truncate(true).open(config_path).await?;
    f.write_all(toml::to_string_pretty(c).unwrap().as_bytes()).await?;
    f.sync_all().await?;
    Ok(())
}
