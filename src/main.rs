mod components;
mod config;
mod sites;

use anyhow::bail;
use dioxus::desktop::Config;
use dioxus::desktop::WindowBuilder;
use dioxus::prelude::*;
use dioxus_icons::lucide::RefreshCcw;
use futures::StreamExt;

use components::button::*;
use components::input::*;
use components::roomcard::*;
use components::tabs::*;

use crate::components::button::Button;
use crate::components::roomcard::CardType::History;
use crate::components::roomcard::CardType::Saved;
use crate::config::ConfigLoader;
use crate::sites::Sites;
use crate::sites::code_to_url_other;

#[derive(Store, PartialEq, Clone, Debug)]
struct RvRoom {
    id: usize,
    code: String,
    owner: Option<String>,
    title: Option<String>,
    cover: Option<String>,
    url: String,
    on_air: bool,
    saved: bool,
    history: bool,
    deleted: bool,
    last_ts: i64,
}

const FAVICON: Asset = asset!("/assets/favicon.ico");
const TAILWIND_CSS: Asset = asset!("/assets/tailwind.css");
const COMPONENTS_CSS: Asset = asset!("/assets/dx-components-theme.css");

fn main() {
    // dioxus::launch(App);
    dioxus::LaunchBuilder::new()
        .with_cfg(desktop! {
           Config::new().with_menu(None).with_window(
               WindowBuilder::new().with_title("Revda").with_decorations(true)
           )
        })
        .launch(App);
}

#[component]
fn App() -> Element {
    rsx! {
        document::Link { rel: "icon", href: FAVICON }
        document::Link { rel: "stylesheet", href: TAILWIND_CSS }
        document::Link { rel: "stylesheet", href: COMPONENTS_CSS }
        Main {}
    }
}

#[component]
pub fn Main() -> Element {
    let mut rv_rooms = use_store(|| Vec::<RvRoom>::new());
    let sites = use_signal(Sites::new);
    let mut rc_input = use_signal(String::new);
    let mut config = use_signal(ConfigLoader::new);
    // let sort_rooms = use_signal(|| false);

    let saved_rooms = use_memo(move || {
        let mut ret: Vec<(usize, usize, bool)> = rv_rooms
            .iter()
            .enumerate()
            .filter_map(|(i, x)| {
                if *x.saved().read() {
                    Some((i, x.id()(), *x.on_air().read()))
                } else {
                    None
                }
            })
            .collect();
        ret.sort_by(|a, b| (b.2, b.1).cmp(&(a.2, a.1)));
        ret
    });

    let mut refresh = use_action(move || async move {
        let mut tasks = futures::stream::FuturesUnordered::new();
        for r in rv_rooms.iter() {
            tasks.push(async move {
                sites.peek().get_room_info(r.into()).await?;
                anyhow::Ok(())
            });
        }
        while let Some(_res) = tasks.next().await {}
        anyhow::Ok(())
    });

    let mut open_new_room = use_action(move || async move {
        if let Ok(u) = config
            .write()
            .add_new_room(rv_rooms, rc_input.read().as_str())
        {
            let Some(r) = rv_rooms.iter().last() else {
                bail!("rv_rooms is empty!")
            };
            sites.peek().get_room_info(r.into()).await?;
            let _ = tokio::process::Command::new("dmlive")
                .arg("--quiet")
                .arg("-u")
                .arg(&u)
                .spawn();
        } else if let Ok(u) = code_to_url_other(rc_input.read().as_str()) {
            let _ = tokio::process::Command::new("dmlive")
                .arg("--quiet")
                .arg("-u")
                .arg(&u)
                .spawn();
        } else {
            tracing::warn!("invalid room code!");
        }
        config.peek().write_config(rv_rooms.into()).await?;
        anyhow::Ok(())
    });

    let sort_rooms = use_action(move || async move {
        rv_rooms.with_mut(|x| x.sort_by_key(|y| y.last_ts));
        config.peek().write_config(rv_rooms.into()).await?;
        anyhow::Ok(())
    });

    use_future(move || async move {
        let _ = config.write().load_config(rv_rooms).await;
        refresh.call();
    });

    rsx! {
        Tabs {
            class: "py-4 px-4",
            gap: "1rem",
            default_value: "tab1".to_string(),
            horizontal: true,
            div {
                class: "grid w-full items-center grid-cols-[1fr_auto_1fr]",
                div {
                    Input {
                        oninput: move |e: FormEvent| { rc_input.set(e.value()) },
                        onkeypress: move |e: KeyboardEvent| {
                            if e.key() == Key::Enter {
                                tracing::info!("{rc_input}");
                                open_new_room.call();
                            }
                        },
                        placeholder: "输入直播间代码",
                        value: rc_input,
                    }
                }
                TabList {
                    class: "justify-self-center",
                    TabTrigger { value: "tab1".to_string(), index: 0usize, "收藏" }
                    TabTrigger { value: "tab2".to_string(), index: 1usize, "历史" }
                }
                div {
                    class: "justify-self-end",
                    Button {
                        onclick: move |_| {
                            refresh.call();
                        },
                        variant: ButtonVariant::Outline,
                        size: ButtonSize::Icon,
                        border_radius: "50%",
                        RefreshCcw {
                            size: "1rem",
                        }
                    }
                }
            }
            TabContent {
                index: 0usize,
                value: "tab1".to_string(),
                padding: "0rem",
                div {
                    class: "grid grid-cols-[repeat(auto-fit,_minmax(16rem,_1fr))] gap-2",
                    for r in saved_rooms.iter().filter_map(|x| rv_rooms.get(x.0)) {
                        RoomCard {
                            key: "{r.id()}",
                            rv_room: r,
                            sites,
                            sort_rooms,
                            card_type: Saved,
                        }
                    }
                }
            }
            TabContent {
                index: 1usize,
                value: "tab2".to_string(),
                padding: "0rem",
                div {
                    class: "grid grid-cols-[repeat(auto-fit,_minmax(16rem,_1fr))] gap-2",
                    for r in rv_rooms.iter().rev().filter(|x| *x.history().read()) {
                        RoomCard {
                            key: "{r.id()}",
                            rv_room: r,
                            sites,
                            sort_rooms,
                            card_type: History,
                        }
                    }
                }
            }
        }
    }
}
