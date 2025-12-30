use reqwest::Client;
use serde::{Deserialize, Serialize};
use tokio::sync::Semaphore;

macro_rules! rvderr {
    ($($args: expr),*) => {
        anyhow::anyhow!("file: {}, line: {}, column: {}", file!(), line!(), column!())
    };
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct RoomInfo {
    pub room_code: String,
    pub cover: String,
    pub title: String,
    pub owner: String,
    pub is_living: bool,
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

    pub async fn get_bilibili_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let info = dmlive::streamfinder::bilibili::get_live_info(&self.client, rid).await?;
        Ok(RoomInfo {
            room_code: format!("bi-{rid}"),
            cover: info.2,
            title: info.1,
            owner: info.0,
            is_living: info.3,
        })
    }

    pub async fn get_douyu_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let info = dmlive::streamfinder::douyu::get_live_info(&self.client, rid).await?;
        Ok(RoomInfo {
            room_code: format!("do-{rid}"),
            cover: info.2,
            title: info.1,
            owner: info.0,
            is_living: info.3,
        })
    }

    pub async fn get_youtube_room_info(&self, rid: &str, tp: u8) -> anyhow::Result<RoomInfo> {
        let u = if tp == 1 {
            format!("https://www.youtube.com/watch?v={}", &rid)
        } else {
            format!("https://www.youtube.com/@{}/live", &rid)
        };
        let info = dmlive::streamfinder::youtube::get_live_info(&self.client, &u).await?;
        Ok(RoomInfo {
            owner: info.0,
            title: info.1,
            cover: info.2,
            is_living: info.3,
            room_code: format!("yt-{}", &info.4),
        })
    }

    pub async fn get_huya_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let url = format!("https://www.huya.com/{}", &rid);
        let info = dmlive::streamfinder::huya::get_live_info(&self.client, &url).await?;
        Ok(RoomInfo {
            room_code: format!("hu-{rid}"),
            cover: info.2,
            title: info.1,
            owner: info.0,
            is_living: info.3,
        })
    }

    pub async fn get_twitch_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let info = dmlive::streamfinder::twitch::get_live_info(&self.client, rid).await?;
        Ok(RoomInfo {
            cover: info.2,
            title: info.1,
            owner: info.0,
            is_living: info.3,
            room_code: format!("tw-{rid}"),
        })
    }

    pub async fn get_room_info_by_code(&self, room_code: &str) -> anyhow::Result<RoomInfo> {
        let ret = if room_code.starts_with("bi-") {
            let _permit = self.semaphore_c.acquire().await?;
            self.get_bilibili_room_info(&room_code[3..]).await?
        } else if room_code.starts_with("do-") {
            let _permit = self.semaphore_c.acquire().await?;
            self.get_douyu_room_info(&room_code[3..]).await?
        } else if room_code.starts_with("hu-") {
            let _permit = self.semaphore_c.acquire().await?;
            self.get_huya_room_info(&room_code[3..]).await?
        } else if room_code.starts_with("yt-") {
            let _permit = self.semaphore_g.acquire().await?;
            self.get_youtube_room_info(&room_code[3..], 0).await?
        } else if room_code.starts_with("ytv-") {
            let _permit = self.semaphore_g.acquire().await?;
            self.get_youtube_room_info(&room_code[4..], 1).await?
        } else if room_code.starts_with("tw-") {
            let _permit = self.semaphore_g.acquire().await?;
            self.get_twitch_room_info(&room_code[3..]).await?
        } else {
            return Err(anyhow::anyhow!("unknown room code"));
        };
        Ok(ret)
    }
}
