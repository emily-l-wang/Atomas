#include "ui.hxx"

static int const circle_rad_ = 18;
static int const grid_width = 1200;
static int const grid_height = 720;
static int const grid_offset_px_ = grid_height/6;
static ge211::Dims<int> const text_shift_ = {-1*grid_offset_px_/16,
                                       grid_offset_px_/5};
static posn const next_atom_pos_ = {grid_width / 2,grid_height * 5/6};
static posn const center_pos_ = next_atom_pos_ - ge211::Dims<int>{0,
                                                               grid_offset_px_};
static posn const score_pos_ = {510, grid_height/3};
static posn const full_pos_ = {510, grid_offset_px_/5};
static posn const end_pos_ = {300, grid_offset_px_ * 3/10};
static posn const end_pos_atom_ = {575, grid_height/5};
static posn const minus_ins_pos_ = {450, grid_height * 9/10};

Ui::Ui(Model& model)
        : model_(model),
          black_plus_back_(circle_rad_, Color(48, 25, 52)),
          black_plus_text_("+", sans_big),
          plus_back_(circle_rad_, Color::medium_red()),
          plus_text_("+", sans_big),
          minus_back_(circle_rad_, Color(0, 75, 100)),
          minus_text_("-", sans_big),
          score_sprite_("Score: 0", sans_big),
          full_sprite_("__ Atoms Until Full", sans),
          congrats_("CONGRATS! You reached the final atom!", sans_big),
          game_over_("Game over! You ended on atom:", sans_big),
          minus_instruct_("Tip: Press the space bar to convert this atom into"
                          " a plus!", sans)
{
    std::ifstream colors("../Resources/elementcolors.txt");
    int a;
    int b;
    int c;
    while (colors >> a >> b >> c)
    {
        color_set_.push_back(Color(a, b, c));
    }

    std::ifstream text("../Resources/elementtext.txt");
    std::string label;
    while (std::getline(text, label))
    {
        // gets rid of the '\r' character at the end
        chomp_cr(label);
        text_set_.push_back(label);
    }

    for (size_t i = 0; i < max_atoms_; i++) {
        background_sprites_.push_back(ge211::Circle_sprite(circle_rad_,
                                                    color_set_[i]));
        text_sprites_.push_back(ge211::Text_sprite(text_set_[i], sans));
    }
}

void
Ui::draw(ge211::Sprite_set& sprites)
{
    score_sprite_.reconfigure(Builder(sans_big) << "Score: " << model_.score());
    sprites.add_sprite(score_sprite_, score_pos_);
    if (model_.is_game_over()) {
        if (model_.win()) {
            sprites.add_sprite(congrats_, end_pos_);
            add_sprite_(sprites, {max_element_, Model::NORM}, end_pos_atom_);
        }
        else {
            sprites.add_sprite(game_over_, end_pos_);
            int max = model_.max_element();
            add_sprite_(sprites, {max, Model::NORM}, end_pos_atom_);
        }
        return;
    }
    size_t i = 0;
    for (auto a : model_.board()) {
        posn temp_pos = board_to_screen_(i);
        add_sprite_(sprites, a, temp_pos);
        i++;
    }
    Model::atom curr = model_.get_current_atom();
    add_sprite_(sprites, curr, next_atom_pos_);

    int f = model_.get_max_atoms() - model_.get_length();
    full_sprite_.reconfigure(Builder(sans) << f << " Atom(s) Until Full");
    sprites.add_sprite(full_sprite_, full_pos_);
}

ge211::Dims<int>
Ui::initial_window_dimensions() const
{
    return {grid_width, grid_height};
}

std::string
Ui::initial_window_title() const
{
    return "Atomas";
}

void
Ui::on_mouse_up(ge211::Mouse_button, posn pos)
{
    int converted = screen_to_board_(pos);
    model_.play_move(converted);
}

void
Ui::on_key(ge211::Key key)
{
    if (key == ge211::Key::code('q')) {
        quit();
    }
    if (key == ge211::Key::code(' ')) {
        model_.play_move(0, true);
    }
}

int
Ui::screen_to_board_(posn physical) const
{
    ge211::Dims<int> dist = physical - center_pos_;
    size_t board_length = model_.get_length();
    if (dist.height > 0) {
        if (dist.width > 0) {
            return 0;
        }
        else {
            return board_length;
        }
    }
    double magnitude = pow(dist.width*dist.width + dist.height*dist.height,0.5);
    int index = ceil((acos(dist.width/magnitude) / M_PI) * (board_length - 1));
    return index;
}

posn
Ui::board_to_screen_(int logical) const
{
    size_t board_length = model_.get_length();
    if (board_length == 1) {
        return center_pos_.right_by(3.5 * grid_offset_px_);
    }
    double angular_dist = logical * (M_PI / (board_length - 1));
    ge211::Dims<int> dist;
    dist = {int(round(3.5 * grid_offset_px_ * cos(angular_dist))),
            -1*int(round(3.5 * grid_offset_px_ * sin(angular_dist)))};
    return center_pos_ + dist;
}

void
Ui::add_sprite_(ge211::Sprite_set& sprites, Model::atom a, posn
pos) const
{
    if (a.type == Model::NORM) {
        sprites.add_sprite(background_sprites_[a.element - 1], pos);
        sprites.add_sprite(text_sprites_[a.element - 1], pos, 2);
    }
    else if (a.type == Model::PLUS) {
        sprites.add_sprite(plus_back_, pos);
        sprites.add_sprite(plus_text_, pos - text_shift_, 2);
    }
    else if (a.type == Model::B_PLUS) {
        sprites.add_sprite(black_plus_back_, pos);
        sprites.add_sprite(black_plus_text_, pos - text_shift_, 2);
    }
    else if (a.type == Model::MINUS && a.element != 0) {
        sprites.add_sprite(background_sprites_[a.element - 1], pos);
        sprites.add_sprite(text_sprites_[a.element - 1], pos, 2);
        sprites.add_sprite(minus_text_, pos, 2);
        sprites.add_sprite(minus_instruct_, minus_ins_pos_);
    }
    else {
        sprites.add_sprite(minus_back_, pos);
        sprites.add_sprite(minus_text_, pos - text_shift_, 2);
    }
}

// Solution from Jesse's answer on Campuswire
void
Ui::chomp_cr(std::string& line)
{
    if (! line.empty() && line.back() == '\r') {
        line.pop_back();
    }
}

