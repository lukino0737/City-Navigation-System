#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFont>
#include <QQuickStyle>
#include <QSurfaceFormat>
#include <core/modules/DataGenerator.h>
#include <api/NavigationAPI.h>
#include <core/DataModel/Graph.h>
#include <gui/MapView.h>

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QSurfaceFormat format;
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);
    app.setFont(QFont("Microsoft YaHei", 10));

    QQuickStyle::setStyle("Basic");

    // 1. 注册 C++ 渲染组件到 QML
    qmlRegisterType<MapView>("Navigation", 1, 0, "MapView");

    // 2. 准备地图数据：默认加载模拟地图
    Graph* globalGraph = new Graph(&app);

    if (!globalGraph->load("map_data.json")) {
        qDebug() << "生成新的模拟地图 (10,000 节点)...";
        generateAndSaveMap(10000, "map_data.json");
        globalGraph->load("map_data.json");
    }

    // 预扫描真实地图目录，供 QML 下拉菜单使用
    globalGraph->refreshAvailableMaps();
    if (globalGraph->availableMaps().isEmpty()) {
        qWarning() << "map_data/ 目录中未找到真实地图文件。"
                       "请使用 scripts/osm_to_json.py 生成地图数据。";
    }

    QQmlApplicationEngine engine;

    // 3. 将 Graph 对象注入 QML 上下文
    engine.rootContext()->setContextProperty("globalGraph", globalGraph);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/city-navigation-system/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
