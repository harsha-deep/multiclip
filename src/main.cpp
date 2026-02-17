#include <gtkmm.h>

class HelloWindow : public Gtk::Window
{
public:
    HelloWindow()
    {
        set_title("Multiclip - Hello GTK3");
        set_default_size(400, 200);

        button.set_label("Hello World");
        button.signal_clicked().connect(sigc::mem_fun(*this, &HelloWindow::on_button_clicked));

        add(button);
        show_all_children();
    }

private:
    Gtk::Button button;

    void on_button_clicked()
    {
        button.set_label("Clicked");
    }
};

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create(argc, argv, "com.harsha.multiclip");

    HelloWindow window;

    return app->run(window);
}
