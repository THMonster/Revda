use anyhow::bail;
use dioxus::prelude::*;
use reqwest::Client;
use tokio::sync::Semaphore;

use crate::{RvRoom, RvRoomStoreExt};

macro_rules! rvderr {
    ($($args: expr),*) => {
        anyhow::anyhow!(
            "file: {}, line: {}, column: {}",
            file!(),
            line!(),
            column!()
        )
    };
}

pub fn code_to_url(code: &str) -> anyhow::Result<String> {
    let Some((pre, rid)) = code.split_once('-') else {
        bail!("bad room code!")
    };
    let url = match pre {
        "bi" => {
            format!("https://live.bilibili.com/{}", rid)
        }
        "do" => {
            format!("https://www.douyu.com/{}", rid)
        }
        "hu" => {
            format!("https://www.huya.com/{}", rid)
        }
        "yt" => {
            format!("https://www.youtube.com/@{}", rid)
        }
        "ytv" => {
            format!("https://www.youtube.com/watch?v={}", rid)
        }
        "tw" => {
            format!("https://www.twitch.tv/{}", rid)
        }
        _ => bail!("unknown room code"),
    };
    Ok(url)
}

pub fn code_to_url_other(code: &str) -> anyhow::Result<String> {
    let url = if code.starts_with("BV") || code.starts_with("av") {
        if let Some((bv, p)) = code.split_once(':') {
            format!("https://www.bilibili.com/video/{}?p={}", bv, p)
        } else {
            format!("https://www.bilibili.com/video/{}", code)
        }
    } else if code.starts_with("ep") || code.starts_with("ss") {
        if let Some((bv, p)) = code.split_once(':') {
            format!("https://www.bilibili.com/bangumi/play/{}?p={}", bv, p)
        } else {
            format!("https://www.bilibili.com/bangumi/play/{}", code)
        }
    } else {
        bail!("unknown code");
    };
    Ok(url)
}

pub struct Sites {
    client: Client,
    semaphore_c: Semaphore,
    semaphore_g: Semaphore,
}

impl Sites {
    pub fn new() -> Self {
        let client = reqwest::Client::builder()
            .connect_timeout(tokio::time::Duration::from_secs(10))
            .build()
            .unwrap();
        Sites {
            client,
            semaphore_c: Semaphore::new(3),
            semaphore_g: Semaphore::new(3),
        }
    }

    pub async fn get_room_info(&self, rv_room: WriteStore<RvRoom>) -> anyhow::Result<()> {
        let Some((pre, rid)) = rv_room
            .code()
            .peek()
            .split_once('-')
            .map(|x| (x.0.to_string(), x.1.to_string()))
        else {
            bail!("bad room code!")
        };
        match pre.as_str() {
            "bi" => {
                let _permit = self.semaphore_c.acquire().await?;
                let info =
                    dmlive::streamfinder::bilibili::get_live_info(&self.client, &rid).await?;
                rv_room.owner().set(Some(info.0));
                rv_room.title().set(Some(info.1));
                rv_room.cover().set(Some(info.2));
                rv_room.on_air().set(info.3);
            }
            "do" => {
                let _permit = self.semaphore_c.acquire().await?;
                let info = dmlive::streamfinder::douyu::get_live_info(&self.client, &rid).await?;
                rv_room.owner().set(Some(info.0));
                rv_room.title().set(Some(info.1));
                rv_room.cover().set(Some(info.2));
                rv_room.on_air().set(info.3);
            }
            "hu" => {
                let _permit = self.semaphore_c.acquire().await?;
                let url = format!("https://www.huya.com/{}", rid);
                let info = dmlive::streamfinder::huya::get_live_info(&self.client, &url).await?;
                rv_room.owner().set(Some(info.0));
                rv_room.title().set(Some(info.1));
                rv_room.cover().set(Some(info.2));
                rv_room.on_air().set(info.3);
            }
            "yt" => {
                let _permit = self.semaphore_g.acquire().await?;
                let u = format!("https://www.youtube.com/@{}/live", &rid);
                let info = dmlive::streamfinder::youtube::get_live_info(&self.client, &u).await?;
                rv_room.owner().set(Some(info.0));
                rv_room.title().set(Some(info.1));
                rv_room.cover().set(Some(info.2));
                rv_room.on_air().set(info.3);
            }
            "ytv" => {
                let _permit = self.semaphore_g.acquire().await?;
                let u = format!("https://www.youtube.com/watch?v={}", &rid);
                let info = dmlive::streamfinder::youtube::get_live_info(&self.client, &u).await?;
                rv_room.owner().set(Some(info.0));
                rv_room.title().set(Some(info.1));
                rv_room.cover().set(Some(info.2));
                rv_room.on_air().set(info.3);
                rv_room.code().set(format!("yt-{}", &info.4));
            }
            "tw" => {
                let _permit = self.semaphore_g.acquire().await?;
                let info = dmlive::streamfinder::twitch::get_live_info(&self.client, &rid).await?;
                rv_room.owner().set(Some(info.0));
                rv_room.title().set(Some(info.1));
                rv_room.cover().set(Some(info.2));
                rv_room.on_air().set(info.3);
            }
            _ => bail!("unknown room code"),
        }
        Ok(())
    }
}
