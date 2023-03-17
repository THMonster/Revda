use anyhow::anyhow;
use reqwest::Client;
use serde::{Deserialize, Serialize};

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
                111, 109
            ),
            ua_mobile: format!("Mozilla/5.0 (Android 10; Mobile; rv:{1}.0) Gecko/{0}.0 Firefox/{0}.0", 111, 109),
        }
    }

    pub async fn get_bilibili_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let j = self
            .client
            .get("https://api.live.bilibili.com/xlive/web-room/v1/index/getH5InfoByRoom")
            .header("User-Agent", &self.ua_mobile)
            .query(&[("room_id", rid)])
            .send()
            .await?
            .json::<serde_json::Value>()
            .await?;
        // println!("{:?}", &j);
        let cover = j.pointer("/data/room_info/keyframe").ok_or(anyhow!("gbri err 1"))?.as_str().unwrap();
        let title = j.pointer("/data/room_info/title").ok_or(anyhow!("gbri err 2"))?.as_str().unwrap();
        let owner = j
            .pointer("/data/anchor_info/base_info/uname")
            .ok_or(anyhow!("gbri err 3"))?
            .as_str()
            .unwrap();
        let is_living = j.pointer("/data/room_info/live_status").ok_or(anyhow!("gbri err 4"))?.as_i64().unwrap();
        Ok(RoomInfo {
            room_code: format!("bi-{}", rid),
            cover: cover.into(),
            title: title.into(),
            owner: owner.into(),
            is_living: if is_living == 1 { true } else { false },
        })
    }

    pub async fn get_douyu_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let j = self
            .client
            .get(format!("https://www.douyu.com/betard/{}", &rid))
            .header("User-Agent", &self.ua)
            .send()
            .await?
            .json::<serde_json::Value>()
            .await?;
        // println!("{:?}", &j);
        let cover = j.pointer("/room/room_pic").ok_or(anyhow!("gdri err 1"))?.as_str().unwrap();
        let title = j.pointer("/room/room_name").ok_or(anyhow!("gdri err 2"))?.as_str().unwrap();
        let owner = j.pointer("/room/nickname").ok_or(anyhow!("gdri err 3"))?.as_str().unwrap();
        let is_living = j.pointer("/room/show_status").ok_or(anyhow!("gdri err 4"))?.as_i64().unwrap();
        let is_living2 = j.pointer("/room/videoLoop").ok_or(anyhow!("gdri err 5"))?.as_i64().unwrap();
        Ok(RoomInfo {
            room_code: format!("do-{}", rid),
            cover: cover.into(),
            title: title.into(),
            owner: owner.into(),
            is_living: if is_living == 1 && is_living2 == 0 { true } else { false },
        })
    }

    pub async fn get_youtube_room_info(&self, rid: &str, tp: u8) -> anyhow::Result<RoomInfo> {
        let u = if tp == 1 {
            format!("https://www.youtube.com/watch?v={}", &rid)
        } else {
            format!("https://www.youtube.com/channel/{}/live", &rid)
        };
        let resp = self
            .client
            .get(u)
            .header("User-Agent", &self.ua)
            .header("accept-language", "en-US")
            .header(
                "Cookie",
                format!("CONSENT=YES+cb.20210810-12-p0.en+FX+{:03}", rand::random::<u64>() % 500),
            )
            .send()
            .await?
            .text()
            .await?;
        // println!("{:?}", &j);
        let re = fancy_regex::Regex::new(r"ytInitialPlayerResponse\s*=\s*(\{.+?\});.*?\</script\>").unwrap();
        let f1 = || -> anyhow::Result<_> {
            let j = re
                .captures(&resp)
                .map(|r| {
                    r.map(|r| {
                        let j: Result<serde_json::Value, _> = serde_json::from_str(r[1].to_string().as_ref());
                        j
                    })
                })?
                .ok_or(anyhow!("gyri err 1"))??;
            let is_living = match j.pointer("/videoDetails/isLive") {
                Some(it) => it.as_bool().ok_or(anyhow!("gyri err 2"))?,
                None => false,
            };
            let tn = j
                .pointer("/videoDetails/thumbnail/thumbnails")
                .ok_or(anyhow!("gyri err 3"))?
                .as_array()
                .unwrap();
            let cover = tn
                .last()
                .ok_or(anyhow!("gyri err 10"))?
                .pointer("/url")
                .ok_or(anyhow!("gyri err 11"))?
                .as_str()
                .unwrap();

            let title = j.pointer("/videoDetails/title").ok_or(anyhow!("gyri err 4"))?.as_str().unwrap();
            let owner = j.pointer("/videoDetails/author").ok_or(anyhow!("gyri err 5"))?.as_str().unwrap();
            let cid = j.pointer("/videoDetails/channelId").ok_or(anyhow!("gyri err 6"))?.as_str().unwrap();

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
            Err(e) => {
                // println!("{}", e);
                let re_cover = fancy_regex::Regex::new(r#"itemprop="thumbnailUrl"\s+href="([^"]+)""#).unwrap();
                let re_cid = fancy_regex::Regex::new(r#"itemprop="channelId"\s+content="([^"]+)""#).unwrap();
                let re_owner = fancy_regex::Regex::new(r#"link\s+itemprop="name"\s+content="([^"]+)""#).unwrap();
                RoomInfo {
                    cover: re_cover.captures(&resp)?.ok_or(anyhow!("gyri err 7"))?[1].to_string(),
                    title: "没有直播标题".into(),
                    owner: re_owner.captures(&resp)?.ok_or(anyhow!("gyri err 8"))?[1].to_string(),
                    is_living: false,
                    room_code: format!("yt-{}", re_cid.captures(&resp)?.ok_or(anyhow!("gyri err 9"))?[1].to_string()),
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
            .ok_or(anyhow!("ghri err 1"))??;
        let j_rd = re2
            .captures(&resp)
            .map(|r| {
                r.map(|r| {
                    let j: Result<serde_json::Value, _> = serde_json::from_str(r[1].to_string().as_ref());
                    j
                })
            })?
            .ok_or(anyhow!("ghri err 7"))??;
        let cover = j_rd
            .pointer("/screenshot")
            .ok_or(anyhow!("ghri err 2"))?
            .as_str()
            .unwrap();
        let avatar = j_pi
            .pointer("/avatar")
            .ok_or(anyhow!("ghri err 6"))?
            .as_str()
            .unwrap();
        let title = j_rd.pointer("/introduction").ok_or(anyhow!("ghri err 3"))?.as_str().unwrap();
        let owner = j_pi.pointer("/nick").ok_or(anyhow!("ghri err 4"))?.as_str().unwrap();
        let is_living = j_rd.pointer("/isOn").ok_or(anyhow!("ghri err 5"))?.as_bool().unwrap();
        Ok(RoomInfo {
            room_code: format!("hu-{}", rid),
            cover: if is_living { cover.into() } else { avatar.into() },
            title: if title.is_empty() { "没有直播标题".into() } else { title.into() },
            owner: owner.into(),
            is_living,
        })
    }

    pub async fn get_twitch_room_info(&self, rid: &str) -> anyhow::Result<RoomInfo> {
        let resp = self
            .client
            .get(format!("https://m.twitch.tv/{}/profile", &rid))
            .header("User-Agent", &self.ua_mobile)
            .header("Accept-Language", "en-US")
            .header("Referer", "https://m.twitch.tv/")
            .send()
            .await?
            .text()
            .await?;
        let re_title = fancy_regex::Regex::new(r#""BroadcastSettings\}\|\{[^"]+":.+?"title":"(.+?)""#).unwrap();
        let re_owner = fancy_regex::Regex::new(r#""User\}\|\{.+?":.+?"displayName":"(.+?)""#).unwrap();
        let re_cover = fancy_regex::Regex::new(r#""Stream\}\|\{.+?":.+?"previewImageURL":"(.+?)""#).unwrap();
        let re_avatar = fancy_regex::Regex::new(r#""User\}\|\{.+?":.+?"profileImageURL.+?":"(.+?)""#).unwrap();
        // let re_status = fancy_regex::Regex::new(r#""User\}\|\{.+?":.+?"stream":null"#).unwrap();

        let owner = re_owner.captures(&resp)?.ok_or(anyhow!("gtri err 1"))?[1].to_string();
        let cover = match re_cover.captures(&resp)? {
            Some(it) => it[1].to_string().replace("{width}x{height}", "320x180"),
            None => re_avatar.captures(&resp)?.ok_or(anyhow!("gtri err 3"))?[1].replace("150x150", "300x300"),
        };
        // println!("{}", &cover);
        let (title, is_living) = match re_title.captures(&resp)? {
            Some(it) => (it[1].to_string(), true),
            None => ("没有直播标题".into(), false),
        };
        Ok(RoomInfo {
            cover,
            title,
            owner,
            is_living,
            room_code: format!("tw-{}", rid),
        })
    }
}
