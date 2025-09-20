#include <gtkmm.h>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <glibmm.h>
#include <atomic>
#include <regex>
#include <memory>
#include <fstream>
#include <filesystem>
#include <vector>
#include <mutex>

namespace fs = std::filesystem;

class DownloaderWindow : public Gtk::ApplicationWindow
{
public:
    DownloaderWindow()
    {
        set_title("YT-DLP GUI Modern");
        set_default_size(500, 600);

        // Cargar la última carpeta seleccionada
        load_last_folder();

        // Configurar el box principal
        main_box.set_orientation(Gtk::Orientation::VERTICAL);
        main_box.set_spacing(10);
        main_box.set_margin(10);
        set_child(main_box);

        // Configurar widgets
        urls_label.set_text("URLs (una por línea):");
        btn_choose_folder.set_label("Seleccionar carpeta");
        btn_download.set_label("Descargar");
        btn_add_url.set_label("+");
        btn_clear_urls.set_label("Limpiar URLs");

        // Configurar el TextView para múltiples URLs
        urls_textview.set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
        scrolled_window.set_child(urls_textview);
        scrolled_window.set_vexpand(true);
        scrolled_window.set_hexpand(true);

        // Configurar ComboBox para calidad de video
        video_quality_combo.append("240p");
        video_quality_combo.append("360p");
        video_quality_combo.append("480p");
        video_quality_combo.append("720p");
        video_quality_combo.append("1080p");
        video_quality_combo.append("1440p");
        video_quality_combo.append("2160p (4K)");
        video_quality_combo.set_active(4); // 1080p por defecto (índice 4)

        // Configurar ComboBox para formato de video
        video_format_combo.append("mp4");
        video_format_combo.append("webm");
        video_format_combo.append("mkv");
        video_format_combo.append("avi");
        video_format_combo.append("mov");
        video_format_combo.append("flv");
        video_format_combo.append("best (formato original)");
        video_format_combo.set_active(0); // mp4 por defecto

        // Configurar ComboBox para calidad de audio
        audio_quality_combo.append("Peor");
        audio_quality_combo.append("128k");
        audio_quality_combo.append("192k");
        audio_quality_combo.append("256k");
        audio_quality_combo.append("320k");
        audio_quality_combo.append("Mejor");
        audio_quality_combo.set_active(4); // 320k por defecto

        // Configurar ComboBox para formato de audio
        audio_format_combo.append("mp3");
        audio_format_combo.append("aac");
        audio_format_combo.append("flac");
        audio_format_combo.append("m4a");
        audio_format_combo.append("opus");
        audio_format_combo.append("vorbis");
        audio_format_combo.append("best (formato original)");
        audio_format_combo.set_active(0); // mp3 por defecto

        // Configurar ComboBox para tipo de descarga
        download_type_combo.append("Video + Audio");
        download_type_combo.append("Solo Video");
        download_type_combo.append("Solo Audio");
        download_type_combo.set_active(0); // Video + Audio por defecto

        // Configurar barra de progreso
        progress_bar.set_show_text(true);
        progress_bar.set_text("0% - Esperando");
        progress_bar.set_fraction(0.0);

        // Configurar label de estado
        status_label.set_text("Listo");
        status_label.set_xalign(0);

        // Crear boxes para botones de URL
        url_buttons_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        url_buttons_box.set_spacing(5);
        url_buttons_box.append(btn_add_url);
        url_buttons_box.append(btn_clear_urls);

        // Crear boxes para etiquetas y combos
        video_quality_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        video_quality_box.set_spacing(10);
        video_quality_label.set_text("Calidad Video:");
        video_quality_box.append(video_quality_label);
        video_quality_box.append(video_quality_combo);

        video_format_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        video_format_box.set_spacing(10);
        video_format_label.set_text("Formato Video:");
        video_format_box.append(video_format_label);
        video_format_box.append(video_format_combo);

        audio_quality_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        audio_quality_box.set_spacing(10);
        audio_quality_label.set_text("Calidad Audio:");
        audio_quality_box.append(audio_quality_label);
        audio_quality_box.append(audio_quality_combo);

        audio_format_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        audio_format_box.set_spacing(10);
        audio_format_label.set_text("Formato Audio:");
        audio_format_box.append(audio_format_label);
        audio_format_box.append(audio_format_combo);

        download_type_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        download_type_box.set_spacing(10);
        download_type_label.set_text("Tipo:");
        download_type_box.append(download_type_label);
        download_type_box.append(download_type_combo);

        // Añadir widgets al box principal
        main_box.append(urls_label);
        main_box.append(scrolled_window);
        main_box.append(url_buttons_box);
        main_box.append(btn_choose_folder);
        main_box.append(download_type_box);
        main_box.append(video_quality_box);
        main_box.append(video_format_box);
        main_box.append(audio_quality_box);
        main_box.append(audio_format_box);
        main_box.append(progress_bar);
        main_box.append(status_label);
        main_box.append(btn_download);

        // Conectar señales
        btn_choose_folder.signal_clicked().connect(sigc::mem_fun(*this, &DownloaderWindow::choose_folder));
        btn_download.signal_clicked().connect(sigc::mem_fun(*this, &DownloaderWindow::start_download));
        btn_add_url.signal_clicked().connect(sigc::mem_fun(*this, &DownloaderWindow::add_url));
        btn_clear_urls.signal_clicked().connect(sigc::mem_fun(*this, &DownloaderWindow::clear_urls));
        download_type_combo.signal_changed().connect(sigc::mem_fun(*this, &DownloaderWindow::on_download_type_changed));

        // Conectar señal de cierre
        signal_hide().connect(sigc::mem_fun(*this, &DownloaderWindow::on_window_hide));

        // Actualizar la interfaz según el tipo de descarga seleccionado
        on_download_type_changed();
    }

    ~DownloaderWindow() override
    {
        // Asegurarse de que todos los hilos de descarga terminen al cerrar la ventana
        stop_download = true;
        std::lock_guard<std::mutex> lock(download_threads_mutex);
        for (auto& thread : download_threads)
        {
            if (thread.joinable())
            {
                thread.detach();
            }
        }
    }

private:
    void load_last_folder()
    {
        std::string config_dir = Glib::get_user_config_dir() + "/yt-dlp-gui";
        std::string config_file = config_dir + "/last_folder.conf";

        // Crear directorio si no existe
        if (!fs::exists(config_dir))
        {
            fs::create_directories(config_dir);
        }

        // Cargar última carpeta si existe
        std::ifstream file(config_file);
        if (file.is_open())
        {
            std::getline(file, folder);
            file.close();

            if (!folder.empty() && fs::exists(folder))
            {
                btn_choose_folder.set_label("Carpeta: " + Glib::path_get_basename(folder));
            }
        }
    }

    void save_last_folder()
    {
        if (folder.empty()) return;

        std::string config_dir = Glib::get_user_config_dir() + "/yt-dlp-gui";
        std::string config_file = config_dir + "/last_folder.conf";

        // Crear directorio si no existe
        if (!fs::exists(config_dir))
        {
            fs::create_directories(config_dir);
        }

        // Guardar última carpeta
        std::ofstream file(config_file);
        if (file.is_open())
        {
            file << folder;
            file.close();
        }
    }

    void choose_folder()
    {
        auto dialog = Gtk::FileChooserNative::create(
                          "Selecciona carpeta de descarga",
                          *this,
                          Gtk::FileChooser::Action::SELECT_FOLDER,
                          "Seleccionar",
                          "Cancelar"
                      );

        // Establecer la última carpeta seleccionada como predeterminada
        if (!folder.empty())
        {
            dialog->set_file(Gio::File::create_for_path(folder));
        }

        dialog->signal_response().connect([this, dialog](int response_id)
        {
            if (response_id == Gtk::ResponseType::ACCEPT)
            {
                auto file = dialog->get_file();
                if (file)
                {
                    folder = file->get_path();
                    btn_choose_folder.set_label("Carpeta: " + Glib::path_get_basename(folder));
                    save_last_folder(); // Guardar la carpeta seleccionada
                }
            }
        });

        dialog->show();
    }

    void add_url()
    {
        auto buffer = urls_textview.get_buffer();
        buffer->insert(buffer->end(), "https://");
    }

    void clear_urls()
    {
        auto buffer = urls_textview.get_buffer();
        buffer->set_text("");
    }

    void on_download_type_changed()
    {
        int download_type = download_type_combo.get_active_row_number();

        // Mostrar u ocultar controles según el tipo de descarga
        if (download_type == 0)   // Video + Audio
        {
            video_quality_box.set_visible(true);
            video_format_box.set_visible(true);
            audio_quality_box.set_visible(true);
            audio_format_box.set_visible(true);
        }
        else if (download_type == 1)   // Solo Video
        {
            video_quality_box.set_visible(true);
            video_format_box.set_visible(true);
            audio_quality_box.set_visible(false);
            audio_format_box.set_visible(false);
        }
        else if (download_type == 2)   // Solo Audio
        {
            video_quality_box.set_visible(false);
            video_format_box.set_visible(false);
            audio_quality_box.set_visible(true);
            audio_format_box.set_visible(true);
        }
    }

    void on_window_hide()
    {
        stop_download = true;
    }

    void start_download()
    {
        if (folder.empty())
        {
            auto dialog = Gtk::AlertDialog::create("Debes seleccionar una carpeta de destino.");
            dialog->show(*this);
            return;
        }

        // Obtener las URLs del TextView
        auto buffer = urls_textview.get_buffer();
        std::string text = buffer->get_text();
        if (text.empty())
        {
            auto dialog = Gtk::AlertDialog::create("Debes introducir al menos una URL.");
            dialog->show(*this);
            return;
        }

        // Dividir el texto en líneas (URLs)
        std::vector<std::string> urls;
        std::istringstream iss(text);
        std::string line;
        while (std::getline(iss, line))
        {
            // Eliminar espacios en blanco al principio y al final
            line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
            line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

            // Ignorar líneas vacías
            if (!line.empty())
            {
                urls.push_back(line);
            }
        }

        if (urls.empty())
        {
            auto dialog = Gtk::AlertDialog::create("No se encontraron URLs válidas.");
            dialog->show(*this);
            return;
        }

        // Deshabilitar el botón de descarga durante la descarga
        btn_download.set_sensitive(false);
        stop_download = false;
        active_downloads = 0;
        completed_downloads = 0;
        total_downloads = urls.size();

        // Actualizar estado
        update_status();

        // Obtener el tipo de descarga seleccionado
        int download_type = download_type_combo.get_active_row_number();

        // Obtener la calidad de video seleccionada
        std::string video_quality;
        switch(video_quality_combo.get_active_row_number())
        {
            case 0: video_quality = "240"; break;
            case 1: video_quality = "360"; break;
            case 2: video_quality = "480"; break;
            case 3: video_quality = "720"; break;
            case 4: video_quality = "1080"; break;
            case 5: video_quality = "1440"; break;
            case 6: video_quality = "2160"; break;
            default: video_quality = "1080"; break;
        }

        // Obtener el formato de video seleccionado
        std::string video_format;
        int video_format_index = video_format_combo.get_active_row_number();
        if (video_format_index == 6)   // "best (formato original)"
        {
            video_format = "best";
        }
        else
        {
            video_format = video_format_combo.get_active_text();
        }

        // Obtener la calidad de audio seleccionada
        std::string audio_quality;
        switch(audio_quality_combo.get_active_row_number())
        {
            case 0: audio_quality = "9"; break; // Peor
            case 1: audio_quality = "5"; break; // 128k
            case 2: audio_quality = "4"; break; // 192k
            case 3: audio_quality = "3"; break; // 256k
            case 4: audio_quality = "0"; break; // 320k
            case 5: audio_quality = "0"; break; // Mejor (igual que 320k)
            default: audio_quality = "0"; break;
        }

        // Obtener el formato de audio seleccionado
        std::string audio_format;
        int audio_format_index = audio_format_combo.get_active_row_number();
        if (audio_format_index == 6)   // "best (formato original)"
        {
            audio_format = "best";
        }
        else
        {
            audio_format = audio_format_combo.get_active_text();
        }

        // Resetear la barra de progreso
        progress_bar.set_fraction(0.0);
        progress_bar.set_text("0% - Iniciando");

        // Ejecutar las descargas en hilos separados
        std::lock_guard<std::mutex> lock(download_threads_mutex);
        download_threads.clear();

        for (const auto& url : urls)
        {
            active_downloads++;
            download_threads.emplace_back([this, url, video_quality, video_format, audio_quality, audio_format, download_type]()
            {
                download_single_url(url, video_quality, video_format, audio_quality, audio_format, download_type);
            });
        }

        // Separar todos los hilos
        for (auto& thread : download_threads)
        {
            thread.detach();
        }
    }

    void download_single_url(const std::string& url, const std::string& video_quality,
                             const std::string& video_format, const std::string& audio_quality,
                             const std::string& audio_format, int download_type)
    {
        try
        {
            // Construir el comando según el tipo de descarga
            std::string cmd = "yt-dlp --newline ";

            if (download_type == 0)   // Video + Audio
            {
                cmd += "-f \"bestvideo[height<=" + video_quality + "]+bestaudio/best[height<=" + video_quality + "]\" ";

                // Añadir parámetro de formato de video si no es "best"
                if (video_format != "best")
                {
                    cmd += "--merge-output-format " + video_format + " ";
                }

                // Añadir parámetros de audio si no es "best"
                if (audio_format != "best")
                {
                    cmd += "--audio-format " + audio_format + " ";
                }

                // Añadir parámetro de calidad de audio
                cmd += "--audio-quality " + audio_quality + " ";
            }
            else if (download_type == 1)   // Solo Video
            {
                cmd += "-f \"bestvideo[height<=" + video_quality + "]\" -S \"res:" + video_quality + "\" ";

                // Añadir parámetro de formato si no es "best"
                if (video_format != "best")
                {
                    cmd += "--merge-output-format " + video_format + " ";
                }
            }
            else if (download_type == 2)   // Solo Audio
            {
                cmd += "-x "; // Extraer audio

                // Añadir parámetro de formato de audio si no es "best"
                if (audio_format != "best")
                {
                    cmd += "--audio-format " + audio_format + " ";
                }

                // Añadir parámetro de calidad de audio
                cmd += "--audio-quality " + audio_quality + " ";
            }

            cmd += "-o \"" + folder + "/%(title)s.%(ext)s\" \"" + url + "\"";

            // Ejecutar el comando y capturar la salida
            FILE* pipe = popen(cmd.c_str(), "r");
            if (!pipe)
            {
                std::cerr << "Error al ejecutar el comando" << std::endl;
                return;
            }

            char buffer[128];
            std::string result = "";

            // Leer la salida del comando línea por línea
            while (!stop_download && fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                result = buffer;

                // Buscar el porcentaje de progreso en la salida
                std::regex progress_regex("(\\d+\\.\\d)%");
                std::smatch match;

                if (std::regex_search(result, match, progress_regex) && match.size() > 1)
                {
                    double progress = std::stod(match[1].str()) / 100.0;

                    // Actualizar la barra de progreso en el hilo principal
                    Glib::signal_idle().connect_once([this, progress]()
                    {
                        if (!stop_download)
                        {
                            progress_bar.set_fraction(progress);
                            progress_bar.set_text(std::to_string(static_cast<int>(progress * 100)) + "% - Descargando");
                        }
                    });
                }
            }

            pclose(pipe);

            // Actualizar contadores
            {
                std::lock_guard<std::mutex> lock(counters_mutex);
                active_downloads--;
                completed_downloads++;
            }

            // Actualizar estado en la UI
            update_status();

            // Verificar si todas las descargas han terminado
            if (completed_downloads == total_downloads && !stop_download)
            {
                // Enviar notificación cuando todas terminen
                std::string type_str;
                if (download_type == 0) type_str = "Videos y Audio";
                else if (download_type == 1) type_str = "Videos";
                else type_str = "Audios";

                std::string notify_cmd = "notify-send -i dialog-information \"Descargas completadas\" \"Se han descargado " +
                                         std::to_string(completed_downloads) + " " + type_str + " correctamente.\"";
                std::system(notify_cmd.c_str());

                // Actualizar la UI en el hilo principal
                Glib::signal_idle().connect_once([this]()
                {
                    progress_bar.set_fraction(1.0);
                    progress_bar.set_text("100% - Completado");
                    btn_download.set_sensitive(true);
                });
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error en el hilo de descarga: " << e.what() << std::endl;
            {
                std::lock_guard<std::mutex> lock(counters_mutex);
                active_downloads--;
            }
            update_status();
        }
    }

    void update_status()
    {
        Glib::signal_idle().connect_once([this]()
        {
            if (!stop_download)
            {
                std::lock_guard<std::mutex> lock(counters_mutex);
                std::string status = "Completados: " + std::to_string(completed_downloads) +
                                     " / Activos: " + std::to_string(active_downloads) +
                                     " / Total: " + std::to_string(total_downloads);
                status_label.set_text(status);

                // Actualizar progreso general
                if (total_downloads > 0)
                {
                    double overall_progress = static_cast<double>(completed_downloads) / total_downloads;
                    progress_bar.set_fraction(overall_progress);
                    progress_bar.set_text(std::to_string(static_cast<int>(overall_progress * 100)) + "% - General");
                }

                // Si no hay descargas activas y todas están completadas, habilitar el botón
                if (active_downloads == 0 && completed_downloads == total_downloads)
                {
                    btn_download.set_sensitive(true);
                }
            }
        });
    }

    // Widgets de la interfaz
    Gtk::Box main_box{Gtk::Orientation::VERTICAL};
    Gtk::Label urls_label;
    Gtk::ScrolledWindow scrolled_window;
    Gtk::TextView urls_textview;
    Gtk::Box url_buttons_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Button btn_add_url;
    Gtk::Button btn_clear_urls;
    Gtk::Button btn_choose_folder;
    Gtk::Button btn_download;
    Gtk::Box video_quality_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Label video_quality_label;
    Gtk::ComboBoxText video_quality_combo;
    Gtk::Box video_format_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Label video_format_label;
    Gtk::ComboBoxText video_format_combo;
    Gtk::Box audio_quality_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Label audio_quality_label;
    Gtk::ComboBoxText audio_quality_combo;
    Gtk::Box audio_format_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Label audio_format_label;
    Gtk::ComboBoxText audio_format_combo;
    Gtk::Box download_type_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Label download_type_label;
    Gtk::ComboBoxText download_type_combo;
    Gtk::ProgressBar progress_bar;
    Gtk::Label status_label;

    // Variables de estado
    std::string folder;
    std::vector<std::thread> download_threads;
    std::mutex download_threads_mutex;
    std::mutex counters_mutex;
    std::atomic<bool> stop_download{false};
    int active_downloads = 0;
    int completed_downloads = 0;
    int total_downloads = 0;
};

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create("org.gtkmm.ytdlp");

    // Conectar la señal activate
    app->signal_activate().connect([&app]()
    {
        auto window = std::make_unique<DownloaderWindow>();
        window->set_visible(true);
        app->add_window(*window);
        window.release();
    });

    return app->run(argc, argv);
}
