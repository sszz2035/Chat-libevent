cmake_minimum_required(VERSION 3.19)
project(chat_client LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)


qt_add_executable(chatclient
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h

    include/ElaWidgetTools.qrc
    src/page-view/landpage.cpp src/page-view/landpage.h src/page-view/registerpage.cpp src/page-view/registerpage.h
    src/utils/net-work/clientconserver.h src/utils/net-work/clientconserver.cpp
    src/page-view/archpage.h src/page-view/archpage.cpp
    src/page-view/addpage.h src/page-view/addpage.cpp
    src/page-view/effect-component/ssmaskwidget.h src/page-view/effect-component/ssmaskwidget.cpp
    src/page-view/effect-component/loadingdialog.h src/page-view/effect-component/loadingdialog.cpp
    src/page-view/creategrouppage.h src/page-view/creategrouppage.cpp
    src/page-view/contactpage.h src/page-view/contactpage.cpp
    src/page-view/noticepage.h src/page-view/noticepage.cpp
    src/page-view/contactdelegate.h src/page-view/contactdelegate.cpp
    src/page-view/contactmodel.h src/page-view/contactmodel.cpp
    src/page-view/aboutpage.h src/page-view/aboutpage.cpp
    src/page-view/messagepage.h src/page-view/messagepage.cpp
    src/page-view/conversationpage.h src/page-view/conversationpage.cpp
    src/page-view/effect-component/msg-bubble/msgbubblemodel.h src/page-view/effect-component/msg-bubble/msgbubblemodel.cpp
    src/page-view/effect-component/msg-bubble/msgbubbledelegate.h src/page-view/effect-component/msg-bubble/msgbubbledelegate.cpp
    src/page-view/effect-component/msg-bubble/msgbubbleview.h src/page-view/effect-component/msg-bubble/msgbubbleview.cpp
    src/page-view/screenshotpage.h src/page-view/screenshotpage.cpp
    src/page-view/emojipickerpage.h src/page-view/emojipickerpage.cpp
    src/page-view/groupmemberdock.h src/page-view/groupmemberdock.cpp
    src/page-view/userpage.h src/page-view/userpage.cpp
    src/page-view/editinfopage.h src/page-view/editinfopage.cpp
    src/page-view/define.h
    src/core/loginverify.h src/core/loginverify.cpp
    src/core/pagedata.h

    src/core/clientrequesthandler.h src/core/clientrequesthandler.cpp
    src/utils/log/logfile.h src/utils/log/logfile.cpp
    src/utils/get-time/GetCurTime.cpp
    src/utils/get-time/GetCurTime.h
    src/core/commondata.h src/core/commondata.cpp
    src/utils/common-func/CommonFunc.h
    src/utils/common-func/CommonFunc.cpp
)

#添加资源文件
# qt6_add_resources(chatclient
#     FILES include/ElaWidgetTools.qrc
# )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql Concurrent)

#添加子目录
add_subdirectory(src/page-view/llm-page)
add_subdirectory(src/core/database)

# 链接库
target_link_libraries(chatclient
    PRIVATE
        Qt::Core
        Qt::Widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/ElaWidgetToolsd.lib
)
target_link_libraries(chatclient PRIVATE LLMChatPagePlugin)
target_link_libraries(chatclient PRIVATE DatabaseLib)
target_link_libraries(chatclient PRIVATE Qt6::Widgets)
target_link_libraries(chatclient PRIVATE Qt6::Core Qt6::Network Qt6::Sql Qt6::Concurrent)
target_link_libraries(chatclient PRIVATE Qt6::Core)
target_link_libraries(chatclient PRIVATE Qt6::Widgets)
target_link_libraries(chatclient PRIVATE Qt6::Core)
target_link_libraries(chatclient PRIVATE Qt6::Core)

include(GNUInstallDirs)

include_directories(${Qt6Core_INCLUDE_DIRS} ${Qt6Network_INCLUDE_DIRS} ${Qt6Sql_INCLUDE_DIRS})
include_directories(
    include
    include/private
    src
    src/core
    src/core/database
)
install(TARGETS chatclient
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET chatclient
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
