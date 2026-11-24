#pragma once
#include <string>
#include "order_list.h"

// Пользовательское меню программы
class Menu {
private:
    OrderList &list;
    bool order_initialized;                                      // Список уже создан или загружен

    std::string ask_line(std::string prompt);                    // Запрос на ввод строки
    int parse_int(std::string s);                                // Преобразование строки в целое число
    int ask_int(std::string prompt);                             // Запрос на ввод целого числа
    int ask_int_in_range(std::string prompt, int min_value, int max_value);
    void read_order_fields(Order &order);                        // Ввод всех полей объекта
    void print_all();                                            // Печать всех записей
    int ask_record_index(std::string action);                    // Выбор записи по номеру (-1 - отмена)
    bool ask_filepath(std::string &filepath, std::string action);// Запрос пути к файлу

    bool startup_dialog();                                       // Создание пустого списка или загрузка
    int ask_order_choice();                                      // Выбор действия в меню заказов

    void run_order_mode();                                       // Работа со списком заказов

    void handle_add_order();                                     // Добавление на любую позицию
    void handle_edit_order();                                    // Редактирование любой записи
    void handle_remove_order();                                  // Удаление любой записи
    void handle_search_orders_by_payer();                        // Сумма по счёту плательщика
    void handle_sort_orders();                                   // Сортировка по счёту плательщика
    void handle_save_orders();                                   // Сохранение в файл
    bool handle_load_orders();                                   // Загрузка из файла
public:
    Menu(OrderList &l);
    ~Menu();
    Menu(const Menu &) = delete;
    Menu& operator=(const Menu &) = delete;

    void run();
};
