use dioxus::prelude::*;
// use dioxus_primitives::{dioxus_attributes::attributes, merge_attributes};

#[css_module("/src/components/skeleton/style.css")]
struct Styles;

#[component]
pub fn Skeleton(
    // class: &'static str,
    class: String,
    #[props(extends=GlobalAttributes)] attributes: Vec<Attribute>,
) -> Element {
    // let base = attributes!(div {
    //     class: Styles::dx_skeleton,
    // });
    // let merged = merge_attributes(vec![base, attributes]);
    //

    let class = format!("bg-bg-c dark:bg-bg-dark-c animate-pulse {}", class);

    rsx! {
        div {
            class,
            ..attributes
        }
    }
}
