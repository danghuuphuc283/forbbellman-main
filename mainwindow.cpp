#include "mainwindow.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QFont>
#include <QInputDialog>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QQueue>
#include <QMessageBox>
#include <QDebug>
#include <cmath>
#include <QVBoxLayout>
#include <QLabel>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    scene(new QGraphicsScene(this)),
    view(new QGraphicsView(scene, this)),
    vertexCounter('A')
{
    onLoadFromFile("C:/learnspace/dirg/project/forbbellman-main/Resources/graph.txt");

    setWindowTitle("Bellman-Ford Visualization");
    resize(800, 600);
    setCentralWidget(view);

    // Kích hoạt khử răng cưa
    view->setRenderHint(QPainter::Antialiasing);

    // Thêm ảnh bản đồ vào scene
    QPixmap mapPixmap(":/resources/map.png");
    if (mapPixmap.isNull()) {
        qDebug("Không tìm thấy ảnh bản đồ, kiểm tra đường dẫn!");
    } else {
        QGraphicsPixmapItem *mapItem = scene->addPixmap(mapPixmap);
        mapItem->setZValue(-1);
    }

    // Tạo nút thêm cạnh
    addEdgeButton = new QPushButton("Thêm cạnh", this);
    addEdgeButton->setGeometry(10, 50, 150, 30);
    addEdgeButton->setStyleSheet("QPushButton { background-color: #FFFACD; color: black; border: 1px solid black; }"
                                 "QPushButton:hover { background-color: #ADD8E6; }");
    connect(addEdgeButton, &QPushButton::clicked, this, &MainWindow::onAddEdge);

    // Tạo nút "Tìm đường đi ngắn nhất"
    QPushButton* findShortestPathButton = new QPushButton("Tìm đường đi ngắn nhất", this);
    findShortestPathButton->setGeometry(10, 100, 150, 30);
    findShortestPathButton->setStyleSheet("QPushButton { background-color: #FFFACD; color: black; border: 1px solid black; }"
                                          "QPushButton:hover { background-color: #ADD8E6; }");
    connect(findShortestPathButton, &QPushButton::clicked, this, &MainWindow::onFindShortestPath);

    // Tạo nút "Đảo dấu trọng số"
    QPushButton* toggleWeightSignButton = new QPushButton("Đảo dấu trọng số", this);
    toggleWeightSignButton->setGeometry(10, 150, 150, 30);
    toggleWeightSignButton->setStyleSheet("QPushButton { background-color: #FFFACD; color: black; border: 1px solid black; }"
                                          "QPushButton:hover { background-color: #ADD8E6; }");
    connect(toggleWeightSignButton, &QPushButton::clicked, this, &MainWindow::onToggleWeightSign);

    QPushButton* exportButton = new QPushButton("Xuất file", this);
    exportButton->setGeometry(10, 200, 150, 30);  // Đặt vị trí và kích thước của nút
    exportButton->setStyleSheet("QPushButton { background-color: #FFFACD; color: black; border: 1px solid black; }"
                                "QPushButton:hover { background-color: #ADD8E6; }");  // Màu nền của nút
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::onExportToFile);

    // Tạo nút "Chụp màn hình và lưu ảnh"
    QPushButton* captureMapButton = new QPushButton("Chụp màn hình bản đồ", this);
    captureMapButton->setGeometry(10, 250, 150, 30);
    connect(captureMapButton, &QPushButton::clicked, this, &MainWindow::onCaptureMap);
    captureMapButton->setStyleSheet("QPushButton { background-color: #FFFACD; color: black; border: 1px solid black; }"
                                    "QPushButton:hover { background-color: #ADD8E6; }");


    QLabel* memberBanner = new QLabel(this);
    memberBanner->setGeometry(10, 300, 220, 60);
    QString bannerText = "Đặng Hữu Phúc                21110071\n"
                         "Võ Thị Huỳnh Hương       19110084\n"
                         "Trần Thanh Luân               21110779";

    memberBanner->setText(bannerText);
    memberBanner->setStyleSheet(
        "background-color: #FFFACD;"    // Màu nền
        "border: 2px solid black;"        // Viền bao bên ngoài
        "font-size: 12px;"                // Cỡ chữ
        "font-weight: bold;"              // Đậm chữ
        "padding: 5px;"                   // Khoảng cách giữa chữ và viền
        );
    memberBanner->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);

    // Tạo khung hiển thị tên chương trình
    QLabel* programTitleBanner = new QLabel(this);


    programTitleBanner->setGeometry(550, 10, 500, 50);

    // Đặt tên chương trình
    programTitleBanner->setText("Tìm đường đi ngắn nhất bằng thuật toán Ford-Bellman");

    // Tùy chỉnh giao diện cho banner
    programTitleBanner->setStyleSheet(
        "background-color: lightblue;"  // Màu nền
        "border: 2px solid black;"      // Viền bao bên ngoài
        "font-size: 16px;"              // Cỡ chữ
        "font-weight: bold;"            // Đậm chữ
        "padding: 5px;"                 // Khoảng cách giữa chữ và viền
        );
    programTitleBanner->setAlignment(Qt::AlignCenter);
}

MainWindow::~MainWindow() {}

// Hàm tính khoảng cách Euclid
double MainWindow::calculateEuclideanDistance(const QPointF& p1, const QPointF& p2) {
    return std::sqrt(std::pow(p1.x() - p2.x(), 2) + std::pow(p1.y() - p2.y(), 2));
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    QPointF sceneMapped = view->mapToScene(event->pos());

    // Thêm đỉnh vào đồ thị
    QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(sceneMapped.x() - 5, sceneMapped.y() - 5, 10, 10);
    ellipse->setBrush(Qt::red);  // Đặt màu đỏ cho chấm

    QChar vertexName = vertexCounter;  // Tạo tên đỉnh (A, B, C,...)
    vertexCounter = QChar(vertexCounter.toLatin1() + 1);  // Tăng giá trị của vertexCounter

    // Lưu lại thông tin đỉnh
    Vertex vertex = { sceneMapped, vertexName, ellipse };
    verticesMap[vertexName] = vertex;

    QGraphicsTextItem* label = new QGraphicsTextItem(vertexName);
    label->setPos(sceneMapped.x() + 10, sceneMapped.y() + 10);  // Đặt tên đỉnh gần vị trí của chấm

    scene->addItem(ellipse);
    scene->addItem(label);
}

void MainWindow::onAddEdge() {
    if (verticesMap.size() < 2) {
        qDebug("Cần ít nhất 2 đỉnh để thêm cạnh.");
        return;
    }

    // Hiển thị hộp thoại để chọn hai đỉnh
    bool ok;
    QString edgeData = QInputDialog::getText(this, "Thêm cạnh", "Nhập hai đỉnh (ví dụ: A B):", QLineEdit::Normal, "", &ok);

    if (!ok || edgeData.isEmpty())
        return;

    // Phân tích dữ liệu nhập
    QStringList parts = edgeData.split(" ");
    if (parts.size() != 2) {
        qDebug("Dữ liệu nhập không hợp lệ. Vui lòng nhập theo định dạng: <đỉnh nguồn> <đỉnh đích>");
        return;
    }

    QChar from = parts[0].at(0).toUpper();
    QChar to = parts[1].at(0).toUpper();

    if (!verticesMap.contains(from) || !verticesMap.contains(to)) {
        qDebug("Một hoặc cả hai đỉnh không tồn tại.");
        return;
    }

    // Tính khoảng cách Euclid làm trọng số
    Vertex fromVertex = verticesMap[from];
    Vertex toVertex = verticesMap[to];
    double distance = calculateEuclideanDistance(fromVertex.position, toVertex.position);
    int weight = static_cast<int>(distance);

    // Vẽ cạnh lên scene
    QGraphicsLineItem* line = scene->addLine(QLineF(fromVertex.position, toVertex.position), QPen(Qt::blue, 5));
    QPointF midpoint = (fromVertex.position + toVertex.position) / 2;

    // Hiển thị trọng số tại trung điểm
    QGraphicsTextItem* textItem = new QGraphicsTextItem(QString::number(weight));
    textItem->setPos(midpoint);
    textItem->setDefaultTextColor(Qt::black);
    scene->addItem(textItem);

    // Lưu thông tin cạnh và trọng số
    edges.append({from, to, weight});
    edges.append({to, from, weight});
}

void MainWindow::onFindShortestPath() {
    bool ok;
    QString source = QInputDialog::getText(this, "Nhập đỉnh nguồn", "Nhập đỉnh nguồn:", QLineEdit::Normal, "", &ok);
    QString target = QInputDialog::getText(this, "Nhập đỉnh đích", "Nhập đỉnh đích:", QLineEdit::Normal, "", &ok);

    if (!ok || source.isEmpty() || target.isEmpty()) return;

    QMap<QChar, int> distance;
    QMap<QChar, QChar> previous;

    for (const QChar& vertex : verticesMap.keys()) {
        distance[vertex] = INT_MAX;
        previous[vertex] = QChar();
    }
    distance[source.at(0)] = 0;

    // Thuật toán Bellman-Ford
    for (int i = 0; i < verticesMap.size() - 1; ++i) {
        for (const Edge& edge : edges) {
            QChar u = edge.from;
            QChar v = edge.to;
            int weight = edge.weight;

            // Cập nhật khoảng cách
            if (distance[u] != INT_MAX && distance[u] + weight < distance[v]) {
                distance[v] = distance[u] + weight;
                previous[v] = u;
            }
        }
    }


    bool hasNegativeCycle = false;
    for (const Edge& edge : edges) {
        QChar u = edge.from;
        QChar v = edge.to;
        int weight = edge.weight;


        if (distance[u] != INT_MAX && distance[u] + weight < distance[v]) {
            // Tìm chu trình âm
            hasNegativeCycle = true;
            break;
        }
    }

    if (hasNegativeCycle) {
        // Kiểm tra và tìm chu trình âm bắt đầu từ nguồn
        QMap<QChar, bool> inCycle;
        QChar current = source.at(0);

        // Dùng thuật toán tìm chu trình âm
        QList<QChar> cycle;
        QChar cycleStart = current;


        while (previous.contains(current) && !inCycle.contains(current)) {
            inCycle[current] = true;
            cycle.append(current);
            current = previous[current];
        }

        // Nếu phát hiện chu trình, thông báo lỗi
        if (inCycle.contains(current)) {
            QMessageBox::critical(this, "Lỗi", "Đồ thị chứa chu trình âm.");
        }
        return;  // Dừng lại và không tiếp tục thực hiện
    }
    for (QGraphicsEllipseItem* item : coloredVertices) {
        item->setBrush(Qt::red);  // Hoặc bất kỳ màu nào bạn muốn sử dụng cho các đỉnh ban đầu
    }
    for (QGraphicsLineItem* line : coloredEdges) {
        line->setPen(QPen(Qt::black, 1));  // Đặt lại màu của các cạnh ban đầu
    }

    // Xóa các phần tử trong danh sách màu cũ
    coloredVertices.clear();
    coloredEdges.clear();

    // Nếu không có chu trình âm, tiếp tục thực hiện tìm đường đi ngắn nhất
    QList<QChar> path;
    QChar current = target.at(0);
    int totalWeight = 0;

    while (current != QChar()) {
        path.prepend(current);
        current = previous[current];
    }

    // Tính tổng trọng số
    for (int i = 0; i < path.size() - 1; ++i) {
        QChar u = path[i];
        QChar v = path[i + 1];
        for (const Edge& edge : edges) {
            if (edge.from == u && edge.to == v) {
                totalWeight += edge.weight;
                break;
            }
        }
    }

    // Chuyển QList<QChar> thành QStringList
    QStringList pathStringList;
    for (const QChar& c : path) {
        pathStringList.append(QString(c));
    }

    QString result = "Đường đi ngắn nhất từ " + source + " đến " + target + ": " + pathStringList.join(" -> ");
    result += "\nTổng trọng số: " + QString::number(totalWeight);
    this->result = result;
    this->totalWeight = totalWeight;

    QMessageBox::information(this, "Kết quả", result);

    // Tô màu các đỉnh và cạnh trên đường đi ngắn nhất
    for (int i = 0; i < path.size(); ++i) {
        QChar vertex = path[i];
        Vertex vertexInfo = verticesMap[vertex];
        vertexInfo.ellipseItem->setBrush(Qt::green);
        coloredVertices.append(vertexInfo.ellipseItem);
    }

    for (int i = 0; i < path.size() - 1; ++i) {
        QChar u = path[i];
        QChar v = path[i + 1];
        QPointF uPos = verticesMap[u].position;
        QPointF vPos = verticesMap[v].position;
        QGraphicsLineItem* line = new QGraphicsLineItem(QLineF(uPos, vPos));
        line->setPen(QPen(Qt::green, 3));
        scene->addItem(line);
        coloredEdges.append(line);
    }
}

void MainWindow::onToggleWeightSign()
{
    if (edges.isEmpty()) {
        QMessageBox::information(this, "Thông báo", "Không có cạnh nào để đảo dấu.");
        return;
    }

    // Hiển thị hộp thoại để chọn hai đỉnh
    bool ok;
    QString edgeData = QInputDialog::getText(
        this, "Đảo dấu trọng số",
        "Nhập hai đỉnh của cạnh cần đổi dấu (ví dụ: A B):",
        QLineEdit::Normal, "", &ok);

    if (!ok || edgeData.isEmpty())
        return;

    // Phân tích dữ liệu nhập
    QStringList parts = edgeData.split(" ");
    if (parts.size() != 2) {
        QMessageBox::warning(this, "Lỗi", "Dữ liệu nhập không hợp lệ. Vui lòng nhập theo định dạng: <đỉnh nguồn> <đỉnh đích>.");
        return;
    }

    QChar from = parts[0].at(0).toUpper();
    QChar to = parts[1].at(0).toUpper();

    // Tìm và đổi dấu trọng số của cạnh
    bool edgeFound = false;
    for (Edge& edge : edges) {
        if ((edge.from == from && edge.to == to) || (edge.from == to && edge.to == from)) {
            edge.weight = -edge.weight;  // Đảo dấu trọng số
            edgeFound = true;

            // Cập nhật hiển thị trọng số trên scene
            for (QGraphicsItem* item : scene->items()) {
                if (auto textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
                    QString text = textItem->toPlainText();
                    bool ok;
                    int weight = text.toInt(&ok);
                    if (ok && weight == -edge.weight) {
                        textItem->setPlainText(QString::number(edge.weight));
                        break;
                    }
                }
            }
            break;
        }
    }

    if (!edgeFound) {
        QMessageBox::warning(this, "Lỗi", "Cạnh không tồn tại trong đồ thị.");
    } else {
        QMessageBox::information(this, "Thành công", "Đã đảo dấu trọng số của cạnh " + QString(from) + " -> " + QString(to) + ".");
    }
}

void MainWindow::onLoadFromFile(const QString& filePath) {
    QFile file(filePath);

    // Mở file để đọc
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Lỗi", "Không thể mở file.");
        return;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    bool ok;

    // Đọc số lượng đỉnh (N)
    int numVertices = line.toInt(&ok);
    if (!ok || numVertices <= 0) {
        QMessageBox::warning(this, "Lỗi", "Số lượng đỉnh không hợp lệ.");
        return;
    }

    verticesMap.clear();
    edges.clear(); // Xóa edges trước khi thêm mới

    // Đọc N dòng tiếp theo để lấy thông tin các đỉnh và tọa độ của chúng
    for (int i = 0; i < numVertices; ++i) {
        line = in.readLine();
        QStringList parts = line.split(" ");

        // Kiểm tra ít nhất có 3 phần tử (Tên đỉnh, X, Y)
        if (parts.size() < 3) {
            QMessageBox::warning(this, "Lỗi", "Dữ liệu đỉnh không hợp lệ.");
            continue;
        }

        QChar vertexName = parts[0].at(0).toUpper();
        qreal x = parts[1].toDouble();
        qreal y = parts[2].toDouble();

        // Tạo đỉnh mới
        QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(x - 5, y - 5, 10, 10);
        ellipse->setBrush(Qt::red); // Đặt màu cho đỉnh
        Vertex vertex = { QPointF(x, y), vertexName, ellipse };
        verticesMap[vertexName] = vertex;

        // Tạo nhãn cho đỉnh
        QGraphicsTextItem* label = new QGraphicsTextItem(vertexName);
        label->setPos(x + 10, y + 10); // Đặt vị trí nhãn gần đỉnh
        scene->addItem(ellipse);
        scene->addItem(label);
    }

    // Đọc các kết nối và trọng số
    while (!in.atEnd()) {
        line = in.readLine();
        QStringList parts = line.split(" ");

        // Kiểm tra ít nhất có 3 phần tử (Tên đỉnh, Tên đỉnh kết nối, Trọng số)
        if (parts.size() < 3) {
            QMessageBox::warning(this, "Lỗi", "Dữ liệu kết nối không hợp lệ.");
            continue;
        }

        QChar vertexName = parts[0].at(0).toUpper();

        // Duyệt qua các đỉnh kết nối và trọng số
        for (int i = 1; i < parts.size(); i += 2) {
            QChar neighbor = parts[i].at(0).toUpper();
            bool validWeight = false;
            int weight = parts[i + 1].toInt(&validWeight);

            if (!validWeight) {
                QMessageBox::warning(this, "Lỗi", "Trọng số không hợp lệ.");
                continue;
            }

            // Kiểm tra nếu đỉnh kết nối chưa tồn tại
            if (!verticesMap.contains(neighbor)) {
                QMessageBox::warning(this, "Lỗi", "Đỉnh kết nối không tồn tại: " + QString(neighbor));
                continue;
            }

            // Vẽ cạnh giữa các đỉnh
            Vertex fromVertex = verticesMap[vertexName];
            Vertex toVertex = verticesMap[neighbor];
            QGraphicsLineItem* lineItem = scene->addLine(QLineF(fromVertex.position, toVertex.position), QPen(Qt::blue, 5));
            QPointF midpoint = (fromVertex.position + toVertex.position) / 2;

            // Hiển thị trọng số của cạnh
            QGraphicsTextItem* textItem = new QGraphicsTextItem(QString::number(weight));
            textItem->setPos(midpoint); // Đặt vị trí trọng số ở giữa cạnh
            textItem->setDefaultTextColor(Qt::black);
            scene->addItem(textItem);

            // Lưu thông tin cạnh
            edges.append({vertexName, neighbor, weight});
            edges.append({neighbor, vertexName, weight}); // Đảm bảo cả 2 chiều
        }
    }

    file.close();
    QMessageBox::information(this, "Thành công", "Đọc dữ liệu từ file thành công.");
}
void MainWindow::onExportToFile() {
    QString result = this->result;
    int totalWeight = this->totalWeight;
    QString filePath = QFileDialog::getSaveFileName(this, "Lưu file", "", "Text Files (*.txt)");
    if (filePath.isEmpty()) return;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Lỗi", "Không thể mở file để lưu.");
        return;
    }
    QTextStream out(&file);
    out << result << "\n";
    file.close();
    QMessageBox::information(this, "Thành công", "Xuất file thành công.");
}
void MainWindow::onCaptureMap() {
    QPixmap pixmap = this->grab();
    QString filePath = QFileDialog::getSaveFileName(this, "Lưu ảnh màn hình", "", "Images (*.png)");
    if (filePath.isEmpty()) return;
    if (pixmap.save(filePath, "PNG")) {
        QMessageBox::information(this, "Thành công", "Ảnh màn hình đã được lưu.");
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể lưu ảnh màn hình.");
    }
}
