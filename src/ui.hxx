#pragma once

#include "model.hxx"
#include <fstream>

using posn = ge211::Posn<int>;
using Color = ge211::Color;
using Builder = ge211::Text_sprite::Builder;

class Ui : public ge211::Abstract_game
{
public:

    Ui(Model&);

protected:

    void on_mouse_up(ge211::Mouse_button, ge211::Posn<int> pos) override;

    void on_key(ge211::Key) override;

    void draw(ge211::Sprite_set&) override;

    ge211::Dims<int> initial_window_dimensions() const override;

    std::string initial_window_title() const override;

private:
    Model& model_;

    int const small_font_size_ = 14;
    int const large_font_size_ = 40;

    ge211::Font sans {"sans.ttf", small_font_size_};
    ge211::Font sans_big {"sans.ttf", large_font_size_};
    size_t const max_atoms_ = model_.get_max_atoms();
    int const max_element_ = model_.get_max_element();

    int screen_to_board_ (ge211::Posn<int> physical) const;

    ge211::Posn<int> board_to_screen_ (int logical) const;

    void add_sprite_(ge211::Sprite_set&, Model::atom, posn) const;

    void chomp_cr(std::string& line);

    std::vector<Color> color_set_;
    std::vector<std::string> text_set_;

    std::vector<ge211::Circle_sprite> background_sprites_;
    std::vector<ge211::Text_sprite> text_sprites_;

    ge211::Circle_sprite black_plus_back_;
    ge211::Text_sprite black_plus_text_;
    ge211::Circle_sprite plus_back_;
    ge211::Text_sprite plus_text_;
    ge211::Circle_sprite minus_back_;
    ge211::Text_sprite minus_text_;
    ge211::Text_sprite score_sprite_;
    ge211::Text_sprite full_sprite_;
    ge211::Text_sprite congrats_;
    ge211::Text_sprite game_over_;
    ge211::Text_sprite minus_instruct_;
};
