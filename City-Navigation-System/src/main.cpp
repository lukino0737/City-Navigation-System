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

    // 2. 准备地图数据
    Graph* globalGraph = new Graph(&app);
    
    // 如果没有地图文件，则生成一个包含 10,000 个节点的默认地图
    if (!globalGraph->load("map_data.json")) {
        qDebug() << "Generating new map data (10,000 nodes)...";
        generateAndSaveMap(10000, "map_data.json");
        globalGraph->load("map_data.json");
    }

    QQmlApplicationEngine engine;

    // 3. 将 Graph 对象注入 QML 上下文
    engine.rootContext()->setContextProperty("globalGraph", globalGraph);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/city-navigation-system/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
