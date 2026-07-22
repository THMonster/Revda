use dioxus::prelude::*;

#[derive(Clone, Copy)]
struct TabsContext {
    current_tab_idx: Signal<usize>,
}

#[component]
pub fn Tabs(children: Element) -> Element {
    let current_tab_idx = use_signal(|| 0);
    use_context_provider(|| TabsContext { current_tab_idx });

    rsx! {
        div {
            class: "flex flex-col items-center",
            { children }
        }
    }
}

#[component]
pub fn TabList(children: Element) -> Element {
    rsx! {
        div {
            class: "flex flex-row gap-1 p-1 h-full rounded-lg bg-bg-b dark:bg-bg-dark-b",
            { children }
        }
    }
}

#[component]
pub fn TabTrigger(index: usize, children: Element) -> Element {
    let mut ctx = use_context::<TabsContext>();
    rsx! {
        button {  
            class: "px-2 py-1 rounded-md cursor-pointer",
            class: if *ctx.current_tab_idx.read() == index { 
                "bg-bg-a dark:bg-bg-dark-a shadow-sm" 
            } else {
                "text-text-b dark:text-text-dark-b hover:text-text-a dark:hover:text-text-dark-a" 
            },
            onclick: move |_| {
                ctx.current_tab_idx.set(index);
            },
            { children }
        }
    }
}

#[component]
pub fn TabContent(index: usize, children: Element) -> Element {
    let ctx = use_context::<TabsContext>();
    rsx! {
        div {
            class: "w-full",
            display: if *ctx.current_tab_idx.read() == index { "block" } else { "none" }, 
            { children }
        }
    }
}
