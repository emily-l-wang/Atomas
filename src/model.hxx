#pragma once

#include <ge211.hxx>
#include <iostream>
#include <vector>
#include <cmath>

class Model
{
public:

    enum atom_type {
        NORM,
        PLUS,
        MINUS,
        B_PLUS
    };

    struct atom {
        int element;
        atom_type type;
    };

    ge211::Random_source<int> rand_atom_;
    ge211::Random_source<int> rand_element_;

    Model();
    // "cheat" parameter allows for artificial setting of the initial
    // conditions so that the win can be reached faster in testing
    explicit Model (bool cheat);

    bool is_game_over() const
    { return game_over_; }

    bool win() const
    { return win_; }

    int score() const
    { return score_; }

    atom get_current_atom() const
    { return next_atom_; }

    size_t get_max_atoms() const
    { return max_atoms_; }

    int get_max_element() const
    { return max_element_; }

    int max_element() const;

    size_t get_length() const
    { return board_.size(); }

    std::vector<atom> board() const
    { return board_; }

    void play_move(size_t index, bool special = false);

#ifdef CS211_TESTING
    friend struct Test_access;
#endif

private:

    bool game_over_ = false;
    bool win_ = false;
    int score_ = 0;
    // atom range starts at Hydrogen (1) to Lithium, inclusive
    int atom_start = 1;
    size_t max_atoms_;
    int max_element_;
    // score needed to have a chance of generating a black plus
    int black_plus_score_;
    std::vector<atom> board_;
    atom next_atom_;

    void generate_next_atom_(bool only_norm = false);
    void reload_atom_range();

    int plus_pos_(bool black);
    void plus_(size_t index, bool black = false);
    void minus_(size_t existing_index, bool spec = false);

};

bool
operator==(Model::atom const& a, Model::atom const& b);

std::ostream&
operator<<(std::ostream& o, Model::atom);

std::ostream&
operator<<(std::ostream& o, std::vector<Model::atom> board);
