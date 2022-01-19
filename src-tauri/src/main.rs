#![cfg_attr(all(not(debug_assertions), target_os = "windows"), windows_subsystem = "windows")]

mod cmd;
mod sites;
mod config;

use std::sync::Arc;

pub struct State {
    config: tokio::sync::RwLock<Option<crate::config::Config>>,
    sites: crate::sites::Sites,
}

fn main() {
    let state = Arc::new(State {
        config: tokio::sync::RwLock::new(None),
        sites: crate::sites::Sites::new(),

    });
    tauri::Builder::default()
        .manage(state)
        .invoke_handler(tauri::generate_handler![
            cmd::init,
            cmd::open_room,
            cmd::get_rooms_saved,
            cmd::add_saved_room,
            cmd::remove_saved_room,
            cmd::get_rooms_history,
            cmd::add_history_room,
            cmd::remove_history_room,
            cmd::get_room_info_by_code
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
