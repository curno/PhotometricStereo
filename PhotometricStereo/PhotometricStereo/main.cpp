#include "stdafx.h"
#include "mainwindow.h"

static string version;
#ifdef DEPLOY
static string base_path = ".\\";
#else
static string base_path = "..\\";
#endif
static string global_css;

static void VersionInfo()
{
    string cmd = base_path + "tools\\VersionInfo\\VersionInfo.exe EXCUTE 1 ..\\tools\\VersionInfo\\"; 
    int code = system(cmd.c_str());
    if (code == 0)
    {
        ifstream ifile(base_path + "tools\\VersionInfo\\Data\\Seq.dat");  
        getline(ifile, version);
    }
}  

static void CSS()
{
    ifstream ifile(base_path + "Data\\style-dark.css");
    global_css = string((std::istreambuf_iterator<char>(ifile)),
        std::istreambuf_iterator<char>());
}
static void GlobalInit()
{
    #ifdef MEMORY_LEAK_DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(29260);
    #endif
    VersionInfo();
    CSS();
}

int main(int argc, char *argv[])
{
    GlobalInit();
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Photometric Stereo By LYF.CURNO");
    w.statusBar()->showMessage(FromStdStringToQString(version));
    w.setWindowIcon(QIcon(":/Icon.png"));
    a.setStyleSheet(FromStdStringToQString(global_css));
    #ifndef DEPLOY
    //w.LoadModel("F:\\Research\\Project\\Data\\Experimentaldata\\Rouned Cube");
    #endif
    w.showMaximized();
    auto retval = a.exec();
    return 0;
}
