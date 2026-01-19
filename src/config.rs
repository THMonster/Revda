use anyhow::bail;
use chrono::Utc;
use dioxus::prelude::*;
use serde::{Deserialize, Serialize};
use std::collections::VecDeque;
use tokio::{fs::OpenOptions, io::AsyncWriteExt};

use crate::{RvRoom, sites};

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Config {
    pub saved: VecDeque<String>,
    pub history: VecDeque<String>,
}

pub struct ConfigLoader {
    next_id: usize,
}

impl ConfigLoader {
    pub fn new() -> Self {
        Self { next_id: 1 }
    }

    pub fn add_new_room(
        &mut self,
        mut rv_rooms: WriteStore<Vec<RvRoom>>,
        code: &str,
    ) -> anyhow::Result<String> {
        let Ok(url) = sites::code_to_url(code) else {
            bail!("bad room code!")
        };
        rv_rooms.with_mut(|x| {
            x.push(RvRoom {
                id: self.next_id,
                code: code.to_string(),
                owner: None,
                title: None,
                cover: None,
                url: url.to_string(),
                on_air: false,
                saved: false,
                history: true,
                deleted: false,
                last_ts: Utc::now().timestamp_millis(),
            });
        });
        self.next_id += 1;
        Ok(url)
    }

    pub async fn load_config(
        &mut self,
        mut rv_rooms: WriteStore<Vec<RvRoom>>,
    ) -> anyhow::Result<()> {
        let proj_dirs = directories::ProjectDirs::from("com", "THMonster", "dmlive").unwrap();
        let d = proj_dirs.config_dir();
        let _ = tokio::fs::create_dir_all(&d).await;
        let config_path = d.join("revda.toml");
        if !config_path.exists() {
            let _ = tokio::fs::File::create(&config_path).await;
        }
        let c = tokio::fs::read(config_path).await?;
        let c = String::from_utf8_lossy(&c);
        let c: Config = toml::from_str(&c).unwrap_or_default();
        let mut rv_rooms = rv_rooms.write();

        for r in c.history {
            if rv_rooms.iter().find(|&x| x.code == r).is_none() {
                let Ok(url) = sites::code_to_url(&r) else {
                    continue;
                };
                rv_rooms.push(RvRoom {
                    id: self.next_id,
                    code: r,
                    owner: None,
                    title: None,
                    cover: None,
                    url,
                    on_air: false,
                    saved: false,
                    history: true,
                    deleted: false,
                    last_ts: 0,
                });
                self.next_id += 1;
            }
        }

        for r in c.saved {
            if rv_rooms
                .iter_mut()
                .find_map(|x| {
                    if x.code == r {
                        x.saved = true;
                        return Some(());
                    }
                    None
                })
                .is_none()
            {
                let Ok(url) = sites::code_to_url(&r) else {
                    continue;
                };
                rv_rooms.push(RvRoom {
                    id: self.next_id,
                    code: r,
                    owner: None,
                    title: None,
                    cover: None,
                    url,
                    on_air: false,
                    saved: true,
                    history: false,
                    deleted: false,
                    last_ts: 0,
                });
                self.next_id += 1;
            }
        }

        Ok(())
    }

    pub async fn write_config(&self, rv_rooms: ReadStore<Vec<RvRoom>>) -> anyhow::Result<()> {
        let proj_dirs = directories::ProjectDirs::from("com", "THMonster", "dmlive").unwrap();
        let mut c = Config {
            saved: VecDeque::new(),
            history: VecDeque::new(),
        };
        let d = proj_dirs.config_dir();
        let _ = std::fs::create_dir_all(&d);
        let config_path = d.join("revda.toml");
        if !config_path.exists() {
            let _ = tokio::fs::File::create(&config_path);
        }
        let mut hist_count = 0;
        for r in rv_rooms.peek().iter().rev() {
            if r.deleted {
                continue;
            }
            if r.saved {
                c.saved.push_front(r.code.to_string());
            }
            if r.history && hist_count < 20 {
                c.history.push_front(r.code.to_string());
                hist_count += 1;
            }
        }
        let mut f = OpenOptions::new()
            .write(true)
            .truncate(true)
            .open(config_path)
            .await?;
        f.write_all(toml::to_string_pretty(&c).unwrap().as_bytes())
            .await?;
        f.sync_all().await?;
        Ok(())
    }
}
