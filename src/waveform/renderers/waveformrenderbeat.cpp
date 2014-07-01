#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>

#include "waveform/renderers/waveformrenderbeat.h"

#include "controlobject.h"
#include "controlobjectthread.h"
#include "track/beats.h"
#include "trackinfoobject.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer),
          m_pBeatActive(NULL) {
    m_beats.resize(128);
}

WaveformRenderBeat::~WaveformRenderBeat() {
    if (m_pBeatActive)
        delete m_pBeatActive;
}

bool WaveformRenderBeat::init() {
    m_pBeatActive = new ControlObjectThread(
            m_waveformRenderer->getGroup(), "beat_active");
    return m_pBeatActive->valid();
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor.setNamedColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor).toRgb();

    m_beatColor4.setNamedColor(context.selectString(node, "BeatColor4"));
    m_beatColor4 = WSkinColor::getCorrectColor(m_beatColor4).toRgb();

    m_beatColor16.setNamedColor(context.selectString(node, "BeatColor16"));
    m_beatColor16 = WSkinColor::getCorrectColor(m_beatColor16).toRgb();

    if (m_beatColor.alphaF() > 0.99)
        m_beatColor.setAlphaF(0.9);

    if (m_beatColor4.alphaF() > 0.99)
        m_beatColor4.setAlphaF(0.9);

    if (m_beatColor16.alphaF() > 0.99)
        m_beatColor16.setAlphaF(0.9);
}

void WaveformRenderBeat::draw(QPainter* painter, QPaintEvent* /*event*/) {
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if (!trackInfo)
        return;

    BeatsPointer trackBeats = trackInfo->getBeats();
    if (!trackBeats)
        return;

    const int trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    const double firstDisplayedPosition = m_waveformRenderer->getFirstDisplayedPosition();
    const double lastDisplayedPosition = m_waveformRenderer->getLastDisplayedPosition();

    // qDebug() << "trackSamples" << trackSamples
    //          << "firstDisplayedPosition" << firstDisplayedPosition
    //          << "lastDisplayedPosition" << lastDisplayedPosition;

    QScopedPointer<BeatIterator> it(trackBeats->findBeats(
        firstDisplayedPosition * trackSamples, lastDisplayedPosition * trackSamples));

    // if no beat do not waste time saving/restoring painter
    if (!it || !it->hasNext()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPen beatPen(m_beatColor);
    beatPen.setWidthF(1.5);
    painter->setPen(beatPen);

    const float rendererHeight = m_waveformRenderer->getHeight();

    int beatCount = 0;

    while (it->hasNext()) {
        double beatPosition = it->next();
        double xBeatPoint = m_waveformRenderer->transformSampleIndexInRendererWorld((int)beatPosition);

        // If we don't have enough space, double the size.
        if (beatCount >= m_beats.size()) {
            m_beats.resize(m_beats.size() * 2);
        }

        int beatNumber = trackBeats->getBeatNumber(beatPosition);
        if (beatNumber % 16 == 0)
            beatPen.setColor(m_beatColor16);
        else if (beatNumber % 4 == 0)
            beatPen.setColor(m_beatColor4);
        else
            beatPen.setColor(m_beatColor);

        //m_beats[beatCount++].setLine(xBeatPoint, 0.0f, xBeatPoint, rendererHeight);
        painter->setPen(beatPen);
        painter->drawLine(xBeatPoint, 0.0f, xBeatPoint, rendererHeight);
    }

    // Make sure to use constData to prevent detaches!
    //painter->drawLines(m_beats.constData(), beatCount);

    painter->restore();
}
