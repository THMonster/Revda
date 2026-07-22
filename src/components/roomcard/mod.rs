use chrono::Utc;
use dioxus::prelude::*;
use dioxus_icons::lucide::{Heart, Trash2};

use crate::{RvRoom, RvRoomStoreExt, components::skeleton::Skeleton, sites::Sites};

#[derive(Clone, Copy, PartialEq)]
pub enum CardType {
    Saved,
    History,
}

#[component]
pub fn RoomCard(
    rv_room: WriteStore<RvRoom>,
    sites: ReadSignal<Sites>,
    sort_rooms: Action<(), ()>,
    card_type: CardType,
) -> Element {
    let mut cover_loaded = use_signal(|| false);
    let mut click = use_action(move || async move {
        let _ = tokio::process::Command::new("dmlive")
            .arg("--quiet")
            .arg("-u")
            .arg(rv_room.url().peek().as_str())
            .spawn();
        let _ = sites.peek().get_room_info(rv_room).await;
        rv_room.last_ts().set(Utc::now().timestamp_millis());
        rv_room.history().set(true);
        sort_rooms.call();
        anyhow::Ok(())
    });

    let mut delete_room = use_action(move || async move {
        match card_type {
            CardType::Saved => rv_room.saved().set(false),
            CardType::History => rv_room.history().set(false),
        }
        sort_rooms.call();
        anyhow::Ok(())
    });

    let mut save_room = use_action(move || async move {
        rv_room.saved().set(true);
        sort_rooms.call();
        anyhow::Ok(())
    });

    rsx! {
        div {
            onclick: move |_| {
                click.call();
            },
            class: "flex flex-col gap-2 -mx-1 rounded-2xl hover:bg-bg-b hover:dark:bg-bg-dark-b transition-all",
            div {
                class: "group/cover relative mx-2 mt-2",
                Skeleton {
                    class: if !cover_loaded() { "absolute aspect-16/10 rounded-xl w-full" } else { "opacity-0" },
                }
                img {
                    class: "object-cover aspect-16/10 w-full rounded-xl",
                    onload: move |_| cover_loaded.set(true),
                    onerror: move |_| {
                        cover_loaded.set(false);
                        rv_room.cover().set(None);
                    },
                    src: rv_room.cover(),
                }
                if rv_room.on_air()() {
                    div {
                        class: "absolute top-1 left-1 px-1 py-0 bg-bg-a/70 dark:bg-bg-dark-a/70",
                        class: "rounded-md border border-primary/50 dark:border-primary-dark/50",
                        class: "text-primary dark:text-primary-dark text-xs backdrop-blur-sm",
                        "直播中"
                    }
                }
                div {
                    class: "absolute flex bottom-1 right-1 gap-1 opacity-0 group-hover/cover:opacity-100 transition-all",
                    if card_type == CardType::History {
                        button {
                            class: "bg-bg-dark-a/80 rounded-md p-1 hover:scale-110 transition-all",
                            onclick: move |e| {
                                e.stop_propagation();
                                save_room.call();
                            },
                            Heart {
                                size: "1.5rem",
                                stroke_width: "1.5",
                                class: "text-text-dark-a",
                            }
                        }
                    }
                    button {
                        class: "bg-bg-dark-a/80 rounded-md p-1 hover:scale-110 transition-all",
                        onclick: move |e| {
                            e.stop_propagation();
                            delete_room.call();
                        },
                        Trash2 {
                            size: "1.5rem",
                            stroke_width: "1.5",
                            class: "text-text-dark-a",
                        }
                    }
                }
            }
            div {
                class: "flex flex-col gap-1 mx-2 mb-1",
                if let Some(owner) = rv_room.owner().as_ref() {
                    p {
                        class: "text-lg truncate",
                        { rv_room.title() }
                    }
                    p {
                        class: "text-sm truncate",
                        "{owner}"
                    }
                } else {
                    Skeleton { class: "w-full rounded-sm h-5 my-1" }
                    Skeleton { class: "w-3/4 rounded-sm h-4 my-0.5" }
                }
            }
        }
    }
}
