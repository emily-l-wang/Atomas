#include "model.hxx"

static int const BLACK_PLUS_SCORE0 = 200;
static int const total_atoms_allowed_ = 20;
static int const highest_atom_num_ = 20;
static int const range_amt_ = 3;
static int const atom_type_gen_ = 30;
static double const inc_score_ = 0.5;
static int const score_bot_ = 40;
static int const cheat_start_ = 15;
static int const cheat_score_ = 45000;

Model::Model()
        : Model(false)
{ }

Model::Model(bool cheat)
    : rand_atom_(atom_type_gen_),
      rand_element_(range_amt_),
      max_atoms_(total_atoms_allowed_),
      max_element_(highest_atom_num_),
      black_plus_score_(BLACK_PLUS_SCORE0)
{
    if (cheat) {
        atom_start = cheat_start_;
        score_ = cheat_score_;
    }
    for (int i = 0; i < 4; i++) {
        board_.push_back({rand_element_.next() + atom_start, NORM});
    }
    generate_next_atom_(true); // starting atom will always be normal type atom
}

void
Model::play_move(size_t index, bool special)
{
    // Playing the move:
    // "special" means the space was pressed on the keyboard, aka the user wants
    // to turn the current atom into a plus (minus atom ability)
    if (special) {
        if (next_atom_.type == MINUS) {
            minus_(index, special);
        }
    }
    else {
        if (next_atom_.type == MINUS) {
            minus_(index);
        }
        else {
            board_.insert(board_.begin() + index, next_atom_);
            generate_next_atom_();
        }
    }

    // process of using plus and black plus to combine atoms
    int plus_exist;
    while ((plus_exist = plus_pos_(false)) > 0) {
        plus_(plus_exist);
    }

    int b_plus_exist = plus_pos_(true);
    if (b_plus_exist > 0) {
        plus_(b_plus_exist, true);
    }

    reload_atom_range();

    // game end conditions
    // reached final atom
    if (max_element() == max_element_) {
        win_ = true;
        game_over_ = true;
        return;
    }
    // filled the board
    if (board_.size() == max_atoms_) {
        game_over_ = true;
    }
}

void
Model::generate_next_atom_(bool only_norm)
{
    if (only_norm) {
        next_atom_ = {rand_element_.next() + atom_start, NORM};
        return;
    }
    int x = rand_atom_.next();
    atom_type type;
    if (x < 20) {
        type = NORM;
    } else if (x < 26) {
        type = PLUS;
    } else if (x < 29) {
        // only generate minus atoms if there's at least two atoms in play
        if (get_length() > 1) {
            type = MINUS;
        }
        else {
            type = NORM;
        }
    } else {
        // only get black plus when over a specific score
        if (score_ >= black_plus_score_) {
            type = B_PLUS;
        }
        else {
            type = NORM;
        }
    }
    if (type == NORM) {
        next_atom_ = {rand_element_.next() + atom_start, type};
    }
    else {
        next_atom_ = {0, type};
    }
}

void
Model::reload_atom_range()
{
    if (score_ > score_bot_) {
        atom_start = floor(log(score_/score_bot_) / inc_score_) + 1;
    }
}

int
Model::plus_pos_(bool black)
{
    if (black) {
        for (size_t index = 1; index < board_.size() - 1; index++) {
            if (board_[index].type == B_PLUS) {
                return index;
            }
        }
    }
    else {
        for (size_t index = 1; index < board_.size() - 1; index++) {
            // third part of the if prevents edge cases like black pluses or
            // pluses being combined
            if (board_[index].type == PLUS &&
                board_[index - 1] == board_[index + 1] &&
                board_[index - 1].type == NORM) {
                return index;
            }
        }
    }
    return -1;
}

void
Model::plus_(size_t index, bool black)
{
    // invariant: index always > 0 and < size - 1
    int range_start = index - 1;
    size_t range_end = index + 1;
    int element_num;
    int start_e = board_[range_start].element;
    int end_e = board_[range_end].element;
    double mult = 1.5;

    if (black) {
        if (start_e > end_e) {
            element_num = start_e + 3;
        }
        else {
            element_num = end_e + 3;
        }
        score_ += floor((start_e + end_e)/ 2);
    }
    else {
        element_num = start_e + 1;
        score_ += floor((start_e + 1) * mult);
    }

    mult += 0.5;

    // for further chain reactions, it's more complicated
    while (range_start - 1 >= 0 && range_end + 1 < board_.size() &&
            board_[range_start - 1] == board_[range_end + 1]) {
        if (board_[range_start - 1].element >= element_num) {
            score_ += floor ((2*board_[range_start - 1].element - element_num +
                    3) * mult);
            element_num += 2;
        }
        else {
            score_ += floor((element_num + 1) * mult);
            element_num++;
        }
        range_start--;
        range_end++;
        mult += 0.5;
    }
    auto it = board_.erase(board_.begin() + range_start,
                           board_.begin() + range_end + 1);
    if (element_num > max_element_) {
        element_num = max_element_; // make sure it doesn't go out of bounds
    }
    board_.insert(it, {element_num, NORM});
}

void
Model::minus_(size_t existing_index, bool spec)
{
    int x = next_atom_.element;
    // element = 0 means that it hasn't chosen a normal atom from the circle
    // to work with yet
    if (x == 0) {
        // This if condition is purely for making life easier for the UI;
        // the difference between board_.size() and board_.size() - 1 on the
        // physical playing board is very minute at times; so this essentially
        // helps the player in case they accidentally click a bit to the side
        if (existing_index == board_.size()) {
            existing_index = board_.size() - 1;
        }
        // only will work if the player clicks a normal atom, does nothing
        // otherwise
        if (board_[existing_index].type == NORM) {
            next_atom_ = {board_[existing_index].element, MINUS};
            board_.erase(board_.begin() + existing_index);
        }
    }
    else {
        if (spec) {
            next_atom_ = {0, PLUS};
        }
        else {
            board_.insert(board_.begin() + existing_index, {x, NORM});
            generate_next_atom_();
        }
    }
}

int
Model::max_element() const
{
    int max_element = 0;
    for (Model::atom a : board_) {
        if (a.element > max_element) {
            max_element = a.element;
        }
    }
    return max_element;
}

bool
operator==(Model::atom const& a, Model::atom const& b) {
    return a.element == b.element && a.type == b.type;
}

std::ostream&
operator<<(std::ostream& o, Model::atom a)
{
    if (a.type == Model::NORM) {
        return o << "NORM" << a.element << " ";
    }
    else if (a.type == Model::PLUS) {
        return o << "PLUS ";
    }
    else if (a.type == Model::MINUS) {
        return o << "MINUS" << a.element << " ";
    }
    else {
        return o << "B_PLUS ";
    }
}

std::ostream&
operator<<(std::ostream& o, std::vector<Model::atom> board) {
    for (auto a : board) {
        o << a;
    }
    return o << "\n";
}

