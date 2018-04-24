#include "mpvemwidget.h"

static void wakeup(void *ctx)
{
    // This callback is invoked from any mpv thread (but possibly also
    // recursively from a thread that is calling the mpv API). Just notify
    // the Qt GUI thread to wake up (so that it can process events with
    // mpv_wait_event()), and return as quickly as possible.
    MpvEmWidget *mpvEmWidget = (MpvEmWidget *)ctx;
    emit mpvEmWidget->mpv_events();
}

void MpvEmWidget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
//    case MPV_EVENT_PROPERTY_CHANGE: {
//        mpv_event_property *prop = (mpv_event_property *)event->data;
//        if (strcmp(prop->name, "time-pos") == 0) {
//            if (prop->format == MPV_FORMAT_DOUBLE) {
//                double time = *(double *)prop->data;
//                std::stringstream ss;
//                ss << "At: " << time;
//                statusBar()->showMessage(QString::fromStdString(ss.str()));
//            } else if (prop->format == MPV_FORMAT_NONE) {
//                // The property is unavailable, which probably means playback
//                // was stopped.
//                statusBar()->showMessage("");
//            }
//        } else if (strcmp(prop->name, "chapter-list") == 0 ||
//                   strcmp(prop->name, "track-list") == 0)
//        {
//            // Dump the properties as JSON for demo purposes.
//#if QT_VERSION >= 0x050000
//            if (prop->format == MPV_FORMAT_NODE) {
//                QVariant v = mpv::qt::node_to_variant((mpv_node *)prop->data);
//                // Abuse JSON support for easily printing the mpv_node contents.
//                QJsonDocument d = QJsonDocument::fromVariant(v);
//                append_log("Change property " + QString(prop->name) + ":\n");
//                append_log(d.toJson().data());
//            }
//#endif
//        }
//        break;
//    }
    case MPV_EVENT_VIDEO_RECONFIG: {
        // Retrieve the new video size.
        int64_t w, h;
        if (mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &w) >= 0 &&
            mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &h) >= 0 &&
            w > 0 && h > 0)
        {
            // Note that the MPV_EVENT_VIDEO_RECONFIG event doesn't necessarily
            // imply a resize, and you should check yourself if the video
            // dimensions really changed.
            // mpv itself will scale/letter box the video to the container size
            // if the video doesn't fit.
//            std::stringstream ss;
//            ss << "Reconfig: " << w << " " << h;
//            statusBar()->showMessage(QString::fromStdString(ss.str()));
        }
        break;
    }
    case MPV_EVENT_LOG_MESSAGE: {
//        struct mpv_event_log_message *msg = (struct mpv_event_log_message *)event->data;
//        std::stringstream ss;
//        ss << "[" << msg->prefix << "] " << msg->level << ": " << msg->text;
//        append_log(QString::fromStdString(ss.str()));
        break;
    }
    case MPV_EVENT_SHUTDOWN: {
        mpv_terminate_destroy(mpv);
        mpv = NULL;
        break;
    }
    default: ;
        // Ignore uninteresting or unknown events.
    }

}

void MpvEmWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;
        handle_mpv_event(event);
    }
}

MpvEmWidget::MpvEmWidget(QWidget *parent, Qt::WindowFlags f, bool cli)
    : QWidget(parent)
{
    mpv = mpv_create();
    if (!mpv)
        throw std::runtime_error("can't create mpv instance");

//    this->setAttribute(Qt::WA_DontCreateNativeAncestors);
//    this->setAttribute(Qt::WA_NativeWindow);

    int64_t wid = this->winId();
    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);
//    mpv_set_option_string(mpv, "input-default-bindings", "yes");
    mpv_set_option_string(mpv, "hwdec", "vaapi");

    connect(this, &MpvEmWidget::mpv_events, this, &MpvEmWidget::on_mpv_events,
            Qt::QueuedConnection);
    mpv_set_wakeup_callback(mpv, wakeup, this);

    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("mpv failed to initialize");


}

void MpvEmWidget::openFile(QString filePath)
{
    if (mpv) {
        const QByteArray c_filename = filePath.toUtf8();
        const char *args[] = {"loadfile", c_filename.data(), NULL};
        mpv_command_async(mpv, 0, args);
    }
}

void MpvEmWidget::command(const QVariant& params)
{
    mpv::qt::command_variant(mpv, params);
}

QVariant MpvEmWidget::getProperty(const QString &name) const
{
    return mpv::qt::get_property_variant(mpv, name);
}

MpvEmWidget::~MpvEmWidget()
{
    if (mpv)
        mpv_terminate_destroy(mpv);
}
