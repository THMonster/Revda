use crate::{config::write_config, sites::RoomInfo};
use std::{collections::VecDeque, sync::Arc};

#[tauri::command]
pub async fn init(state: tauri::State<'_, Arc<crate::State>>) -> Result<(), String> {
    let mut config = state.config.write().await;
    let proj_dirs = directories::ProjectDirs::from("com", "THMonster", "dmlive").unwrap();
    let d = proj_dirs.config_dir();
    let _ = tokio::fs::create_dir_all(&d).await;
    let config_path = d.join("revda.toml");
    if !config_path.exists() {
        let _ = tokio::fs::File::create(&config_path).await;
    }
    let c = tokio::fs::read(config_path).await.map_err(|e| e.to_string())?;
    let c = String::from_utf8_lossy(&c);
    let c = match crate::config::load_config(&c) {
        Ok(it) => it,
        Err(_) => crate::config::Config {
            saved: VecDeque::new(),
            history: VecDeque::new(),
        },
    };
    *config = Some(c);
    Ok(())
}

#[tauri::command]
pub async fn open_room(room_code: String) -> Result<(), String> {
    let url = if room_code.starts_with("bi-") {
        format!("https://live.bilibili.com/{}", &room_code[3..])
    } else if room_code.starts_with("do-") {
        format!("https://www.douyu.com/{}", &room_code[3..])
    } else if room_code.starts_with("hu-") {
        format!("https://www.huya.com/{}", &room_code[3..])
    } else if room_code.starts_with("yt-") {
        format!("https://www.youtube.com/@{}", &room_code[3..])
    } else if room_code.starts_with("ytv-") {
        format!("https://www.youtube.com/watch?v={}", &room_code[4..])
    } else if room_code.starts_with("tw-") {
        format!("https://www.twitch.tv/{}", &room_code[3..])
    } else if room_code.starts_with("BV") || room_code.starts_with("av") {
        let sp: Vec<&str> = room_code.split(':').collect();
        if sp.len() == 2 {
            format!("https://www.bilibili.com/video/{}?p={}", sp[0], sp[1])
        } else {
            format!("https://www.bilibili.com/video/{}", &room_code)
        }
    } else if room_code.starts_with("ep") || room_code.starts_with("ss") {
        let sp: Vec<&str> = room_code.split(':').collect();
        if sp.len() == 2 {
            format!("https://www.bilibili.com/bangumi/play/{}?p={}", sp[0], sp[1])
        } else {
            format!("https://www.bilibili.com/bangumi/play/{}", &room_code)
        }
    } else {
        return Err("unknown room code!".into());
    };
    let _ = tokio::process::Command::new("dmlive").arg("--quiet").arg("-u").arg(url).spawn();
    Ok(())
}

#[tauri::command]
pub async fn get_rooms_saved(state: tauri::State<'_, Arc<crate::State>>) -> Result<VecDeque<String>, String> {
    Ok(state.config.read().await.as_ref().unwrap().saved.clone())
}

#[tauri::command]
pub async fn add_saved_room(state: tauri::State<'_, Arc<crate::State>>, room_code: String) -> Result<(), String> {
    let mut c = state.config.write().await;
    match c.as_ref().unwrap().saved.iter().position(|x| x.eq(&room_code)) {
        Some(_) => {}
        None => {
            c.as_mut().unwrap().saved.push_back(room_code);
        }
    };
    write_config(c.as_ref().unwrap()).await.map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
pub async fn remove_saved_room(state: tauri::State<'_, Arc<crate::State>>, room_code: String) -> Result<(), String> {
    let mut c = state.config.write().await;
    match c.as_ref().unwrap().saved.iter().position(|x| x.eq(&room_code)) {
        Some(it) => {
            c.as_mut().unwrap().saved.remove(it);
        }
        None => {}
    };
    write_config(c.as_ref().unwrap()).await.map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
pub async fn get_rooms_history(state: tauri::State<'_, Arc<crate::State>>) -> Result<VecDeque<String>, String> {
    Ok(state.config.read().await.as_ref().unwrap().history.clone())
}

#[tauri::command]
pub async fn add_history_room(state: tauri::State<'_, Arc<crate::State>>, room_code: String) -> Result<(), String> {
    let mut c = state.config.write().await;
    match c.as_ref().unwrap().history.iter().position(|x| x.eq(&room_code)) {
        Some(it) => {
            c.as_mut().unwrap().history.remove(it);
        }
        None => {}
    };
    c.as_mut().unwrap().history.push_back(room_code);
    if c.as_ref().unwrap().history.len() > 36 {
        c.as_mut().unwrap().history.pop_front();
    }
    write_config(c.as_ref().unwrap()).await.map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
pub async fn remove_history_room(state: tauri::State<'_, Arc<crate::State>>, room_code: String) -> Result<(), String> {
    let mut c = state.config.write().await;
    match c.as_ref().unwrap().history.iter().position(|x| x.eq(&room_code)) {
        Some(it) => {
            c.as_mut().unwrap().history.remove(it);
        }
        None => {}
    };
    write_config(c.as_ref().unwrap()).await.map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
pub async fn get_room_info_by_code(state: tauri::State<'_, Arc<crate::State>>, room_code: String) -> Result<RoomInfo, String> {
    // let ret = if room_code.starts_with("bi-") {
    //     state.sites.get_bilibili_room_info(&room_code[3..]).await.map_err(|e| e.to_string())?
    // } else if room_code.starts_with("do-") {
    //     state.sites.get_douyu_room_info(&room_code[3..]).await.map_err(|e| e.to_string())?
    // } else if room_code.starts_with("hu-") {
    //     state.sites.get_huya_room_info(&room_code[3..]).await.map_err(|e| e.to_string())?
    // } else if room_code.starts_with("yt-") {
    //     state.sites.get_youtube_room_info(&room_code[3..], 0).await.map_err(|e| e.to_string())?
    // } else if room_code.starts_with("ytv-") {
    //     state.sites.get_youtube_room_info(&room_code[4..], 1).await.map_err(|e| e.to_string())?
    // } else if room_code.starts_with("tw-") {
    //     state.sites.get_twitch_room_info(&room_code[3..]).await.map_err(|e| e.to_string())?
    // } else {
    //     return Err("unknown room code!".into());
    // };
    let ret = state.sites.get_room_info_by_code(&room_code).await.map_err(|e| e.to_string())?;
    Ok(ret)
}
