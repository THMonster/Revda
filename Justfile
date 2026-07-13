build-dmlive:
	cd dmlive && cargo build --release

build-revda:
	dx build -r

build-all: build-revda build-dmlive

[arg("dir", long="prefix-dir", short="d")]
install-dmlive dir: build-dmlive
	install -Dm755 dmlive/target/release/dmlive -t "{{dir}}/bin/"

[arg("dir", long="prefix-dir", short="d")]
install-revda dir: build-revda
	install -vDm 755 "target/dx/revda/release/linux/app/revda" -t "{{dir}}/bin/"
	install -vDm 644 target/dx/revda/release/linux/app/assets/*.css -t "{{dir}}/lib/revda/assets/"
	install -vDm 644 "misc/icon/revda.svg" -t "{{dir}}/share/icons/hicolor/scalable/apps"
	install -vDm 644 "misc/revda.desktop" -t "{{dir}}/share/applications"
	install -vDm 644 "misc/dmlive-mime.desktop" -t "{{dir}}/share/applications"

[arg("dir", long="prefix-dir", short="d")]
install-all dir: (install-dmlive dir) (install-revda dir)
