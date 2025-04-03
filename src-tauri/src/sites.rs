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
    ua: String,
    ua_mobile: String,
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
            ua: format!(
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:{1}.0) Gecko/20100101 Firefox/{0}.0",
                136, 136
            ),
            ua_mobile: format!("Mozilla/5.0 (Android 10; Mobile; rv:{1}.0) Gecko/{0}.0 Firefox/{0}.0", 136, 136),
            semaphore_c: Semaphore::new(3),
            semaphore_g: Semaphore::new(3),
        }
    }

    pub async fn get_bilibili_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let resp = self
            .client
            .get(format!("https://live.bilibili.com/{}", rid))
            .header("User-Agent", &self.ua)
            .header("Connection", "keep-alive")
            .send()
            .await?
            .text()
            .await?;
        let re = fancy_regex::Regex::new(r"window.__NEPTUNE_IS_MY_WAIFU__\s*=\s*(\{.+?\})\s*</script>").unwrap();
        if let Some(cap) = re.captures(&resp)? {
            let j: serde_json::Value = serde_json::from_str(cap[1].to_string().as_ref())?;
            // println!("{:#?}", &j);
            let cover = j.pointer("/roomInfoRes/data/room_info/cover").ok_or_else(|| rvderr!())?.as_str().unwrap();
            let keyframe = j
                .pointer("/roomInfoRes/data/room_info/keyframe")
                .ok_or_else(|| rvderr!())?
                .as_str()
                .unwrap();
            let title = j.pointer("/roomInfoRes/data/room_info/title").ok_or_else(|| rvderr!())?.as_str().unwrap();
            let owner = j
                .pointer("/roomInfoRes/data/anchor_info/base_info/uname")
                .ok_or_else(|| rvderr!())?
                .as_str()
                .unwrap();
            let is_living = j
                .pointer("/roomInfoRes/data/room_info/live_status")
                .ok_or_else(|| rvderr!())?
                .as_i64()
                .unwrap();
            Ok(RoomInfo {
                room_code: format!("bi-{}", rid),
                cover: if keyframe.is_empty() { cover.to_string() } else { keyframe.to_string() },
                title: title.to_string(),
                owner: owner.to_string(),
                is_living: if is_living == 1 { true } else { false },
            })
        } else {
            let mut param1 = Vec::new();
            param1.push(("room_ids", rid));
            param1.push(("req_biz", "web_room_componet"));
            let resp = self
                .client
                .get("https://api.live.bilibili.com/xlive/web-room/v1/index/getRoomBaseInfo")
                .query(&param1)
                .send()
                .await?
                .json::<serde_json::Value>()
                .await?;
            let j = resp
                .pointer("/data/by_room_ids")
                .ok_or_else(|| rvderr!())?
                .as_object()
                .ok_or_else(|| rvderr!())?;
            let j = j.iter().next().ok_or_else(|| rvderr!())?.1;
            let bg = j.pointer("/background").ok_or_else(|| rvderr!())?.as_str().unwrap();
            let cover = j.pointer("/cover").ok_or_else(|| rvderr!())?.as_str().unwrap();
            let title = j.pointer("/title").ok_or_else(|| rvderr!())?.as_str().unwrap();
            let owner = j.pointer("/uname").ok_or_else(|| rvderr!())?.as_str().unwrap();
            let is_living = j.pointer("/live_status").ok_or_else(|| rvderr!())?.as_i64().unwrap();
            Ok(RoomInfo {
                room_code: format!("bi-{}", rid),
                cover: if cover.is_empty() { bg.to_string() } else { cover.to_string() },
                title: title.to_string(),
                owner: owner.to_string(),
                is_living: if is_living == 1 { true } else { false },
            })
        }
    }

    pub async fn get_douyu_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let j = self
            .client
            .get(format!("https://www.douyu.com/betard/{}", &rid))
            .header("User-Agent", &self.ua)
            .header("Connection", "keep-alive")
            .send()
            .await?
            .json::<serde_json::Value>()
            .await?;
        // println!("{:?}", &j);
        let cover = j.pointer("/room/room_pic").ok_or_else(|| rvderr!())?.as_str().unwrap();
        let title = j.pointer("/room/room_name").ok_or_else(|| rvderr!())?.as_str().unwrap();
        let owner = j.pointer("/room/nickname").ok_or_else(|| rvderr!())?.as_str().unwrap();
        let is_living = j.pointer("/room/show_status").ok_or_else(|| rvderr!())?.as_i64().unwrap();
        let is_living2 = j.pointer("/room/videoLoop").ok_or_else(|| rvderr!())?.as_i64().unwrap();
        Ok(RoomInfo {
            room_code: format!("do-{}", rid),
            cover: cover.into(),
            title: title.into(),
            owner: owner.into(),
            is_living: if is_living == 1 && is_living2 == 0 { true } else { false },
        })
    }

    pub async fn get_youtube_room_info(&self, rid: &str, tp: u8) -> anyhow::Result<RoomInfo> {
        let mut cid = "null".to_string();
        let u = if tp == 1 {
            format!("https://www.youtube.com/watch?v={}", &rid)
        } else {
            cid = rid.to_string();
            format!("https://www.youtube.com/@{}/live", &rid)
        };
        let resp = self
            .client
            .get(u)
            .header("User-Agent", &self.ua)
            .header("accept-language", "en-US")
            .header("Connection", "keep-alive")
            // .version(reqwest::Version::HTTP_2)
            // .header(
            //     "Cookie",
            //     format!("CONSENT=YES+cb.20210810-12-p0.en+FX+{:03}", rand::random::<u64>() % 500),
            // )
            .send()
            .await?
            .text()
            .await?;
        // println!("{:?}", &resp);
        let re = fancy_regex::Regex::new(r"ytInitialPlayerResponse\s*=\s*(\{.+?\});.*?</script>").unwrap();
        let mut f1 = || -> anyhow::Result<_> {
            let j = re
                .captures(&resp)
                .map(|r| {
                    r.map(|r| {
                        let j: Result<serde_json::Value, _> = serde_json::from_str(r[1].to_string().as_ref());
                        j
                    })
                })?
                .ok_or_else(|| rvderr!())??;

            cid = j
                .pointer("/microformat/playerMicroformatRenderer/ownerProfileUrl")
                .ok_or_else(|| rvderr!())?
                .as_str()
                .ok_or_else(|| rvderr!())?
                .split('/')
                .last()
                .ok_or_else(|| rvderr!())?
                .strip_prefix("@")
                .ok_or_else(|| rvderr!())?
                .to_string();

            let is_living = match j.pointer("/videoDetails/isLive") {
                Some(it) => it.as_bool().ok_or_else(|| rvderr!())?,
                None => false,
            };
            let tn = j
                .pointer("/videoDetails/thumbnail/thumbnails")
                .ok_or_else(|| rvderr!())?
                .as_array()
                .unwrap();
            let cover = tn
                .last()
                .ok_or_else(|| rvderr!())?
                .pointer("/url")
                .ok_or_else(|| rvderr!())?
                .as_str()
                .unwrap();

            let title = j.pointer("/videoDetails/title").ok_or_else(|| rvderr!())?.as_str().unwrap();
            let owner = j.pointer("/videoDetails/author").ok_or_else(|| rvderr!())?.as_str().unwrap();

            Ok(RoomInfo {
                cover: cover.into(),
                title: title.into(),
                owner: owner.into(),
                is_living,
                room_code: format!("yt-{}", cid),
            })
        };
        let ret = match f1() {
            Ok(it) => it,
            Err(_e) => {
                // println!("{}", e);
                let re_cover = fancy_regex::Regex::new(r#"link\s+rel="image_src"\s+href="([^"]+)""#).unwrap();
                let re_owner = fancy_regex::Regex::new(r#"meta\s+property="og:title"\s+content="([^"]+)""#).unwrap();
                RoomInfo {
                    cover: re_cover.captures(&resp)?.ok_or_else(|| rvderr!())?[1].to_string(),
                    title: "没有直播标题".into(),
                    owner: re_owner.captures(&resp)?.ok_or_else(|| rvderr!())?[1].to_string(),
                    is_living: false,
                    room_code: format!("yt-{}", cid),
                }
            }
        };
        Ok(ret)
    }

    pub async fn get_huya_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let resp = self
            .client
            .get(format!("https://www.huya.com/{}", &rid))
            .header("User-Agent", &self.ua)
            .header("Connection", "keep-alive")
            // .version(reqwest::Version::HTTP_2)
            .send()
            .await?
            .text()
            .await?;
        // println!("{}", &resp);
        // let re = fancy_regex::Regex::new(r"HNF_GLOBAL_INIT\s*=\s*(\{.+?\});*\s*\</script\>").unwrap();
        let re1 = fancy_regex::Regex::new(r"var\s+TT_PROFILE_INFO\s+=\s+(.+\});").unwrap();
        let re2 = fancy_regex::Regex::new(r"var\s+TT_ROOM_DATA\s+=\s+(.+\});").unwrap();
        let j_pi = re1
            .captures(&resp)
            .map(|r| {
                r.map(|r| {
                    let j: Result<serde_json::Value, _> = serde_json::from_str(r[1].to_string().as_ref());
                    j
                })
            })?
            .ok_or_else(|| rvderr!())??;
        let j_rd = re2
            .captures(&resp)
            .map(|r| {
                r.map(|r| {
                    let j: Result<serde_json::Value, _> = serde_json::from_str(r[1].to_string().as_ref());
                    j
                })
            })?
            .ok_or_else(|| rvderr!())??;
        let cover = j_rd.pointer("/screenshot").ok_or_else(|| rvderr!())?.as_str().unwrap();
        let avatar = j_pi.pointer("/avatar").ok_or_else(|| rvderr!())?.as_str().unwrap();
        let title = j_rd.pointer("/introduction").ok_or_else(|| rvderr!())?.as_str().unwrap();
        let owner = j_pi.pointer("/nick").ok_or_else(|| rvderr!())?.as_str().unwrap();
        let is_living = j_rd.pointer("/isOn").ok_or_else(|| rvderr!())?.as_bool().unwrap();
        Ok(RoomInfo {
            room_code: format!("hu-{}", rid),
            cover: if is_living { cover.into() } else { avatar.into() },
            title: if title.is_empty() { "没有直播标题".into() } else { title.into() },
            owner: owner.into(),
            is_living,
        })
    }

    pub async fn get_twitch_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let f1 = |html: &str| {
            let re = fancy_regex::Regex::new(r#"".+<script type="application/ld\+json">(.+?)</script>.+""#).unwrap();
            let j = re.captures(html)?.ok_or_else(|| rvderr!())?[1].to_string();
            let j: serde_json::Value = serde_json::from_str(&j)?;
            let title = j
                .pointer("/@graph/0/description")
                .ok_or_else(|| rvderr!())?
                .as_str()
                .ok_or_else(|| rvderr!())?
                .to_string();
            let owner = j
                .pointer("/@graph/0/name")
                .ok_or_else(|| rvderr!())?
                .as_str()
                .ok_or_else(|| rvderr!())?
                .to_string();
            let cover = j
                .pointer("/@graph/0/thumbnailUrl/2")
                .ok_or_else(|| rvderr!())?
                .as_str()
                .ok_or_else(|| rvderr!())?
                .to_string();
            anyhow::Ok((title, owner, cover, true))
        };
        let f2 = |html: &str| {
            let re_owner = fancy_regex::Regex::new(r#"".+<meta property="og:title" content="(.+?)"/>.+""#).unwrap();
            let re_owner_b = fancy_regex::Regex::new(r#"".+<meta content="(.+?)" property="og:title"/>.+""#).unwrap();
            let re_cover = fancy_regex::Regex::new(r#"".+<meta property="og:image" content="(.+?)"/>.+""#).unwrap();
            let re_cover_b = fancy_regex::Regex::new(r#"".+<meta content="(.+?)" property="og:image"/>.+""#).unwrap();
            let owner = match re_owner.captures(html)? {
                Some(it) => it[1].to_string(),
                None => re_owner_b.captures(html)?.ok_or_else(|| rvderr!())?[1].to_string(),
            };
            let cover = match re_cover.captures(html)? {
                Some(it) => it[1].to_string(),
                None => re_cover_b.captures(html)?.ok_or_else(|| rvderr!())?[1].to_string(),
            };
            anyhow::Ok(("没有直播标题".to_string(), owner, cover, false))
        };
        let resp = self
            .client
            .get(format!("https://www.twitch.tv/{}", &rid))
            .header("User-Agent", &self.ua)
            // .header("User-Agent", "curl/8.8.0")
            // .header("Accept-Language", "en-US")
            // .header("Referer", "https://www.twitch.tv/")
            // .header("Connection", "keep-alive")
            .version(reqwest::Version::HTTP_11)
            .send()
            .await?
            .text()
            .await?;
        // println!("{}", &resp);

        let (title, owner, cover, is_living) = match f1(&resp) {
            Ok(it) => it,
            Err(_) => f2(&resp)?,
        };

        let owner = match owner.strip_suffix(" - Twitch") {
            Some(it) => it.to_string(),
            None => owner,
        };
        Ok(RoomInfo {
            cover,
            title,
            owner,
            is_living,
            room_code: format!("tw-{}", rid),
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
