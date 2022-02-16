#include "model.hxx"
#include <catch.hxx>

struct Test_access
{
    Model &model;

    void
    change_b_plus_score_ (int x) {
        model.black_plus_score_ = x;
    }
};

TEST_CASE("Plus Atom Combining Instantly")
{
    Model m;
    // N (norm), N, N, N, Plus, Plus, Plus, Plus
    std::vector<int> element_type = {10, 10, 10, 10, 21, 21, 21, 21};
    // one less because it doesn't use element_num when a special atom
    std::vector<int> element_num = {2, 2, 4, 4}; // Li, Li, B, Li
    m.rand_atom_.stub_with(element_type);
    m.rand_element_.stub_with(element_num);

    m.play_move(0); // board: rand rand rand rand rand
    m.play_move(2); // board: rand rand Li rand rand rand

    // should have lithium in the next_atom
    CHECK(m.board()[2].type == Model::NORM);
    CHECK(m.board()[2].element == 3);
    CHECK(m.get_current_atom().type == Model::NORM);
    CHECK(m.get_current_atom().element == 3);

    m.play_move(2); // board: rand rand Li Li rand rand rand
    m.play_move(2); // board: rand rand B Li Li rand rand rand
    m.play_move(5); // board: rand rand B Li Li B rand rand rand
    CHECK(m.score() >= 0);
    CHECK(m.get_length() == 9);
    m.play_move(4); // board: rand rand B Li + Li B rand rand rand
    // at most five random atoms + 1 combined atom --> at most 6 atoms
    CHECK(m.get_length() < 7);
    // should get at least 6 points for combining Li Li, then 18 for B B
    // (10 - 4 + 3) * 2 = 18
    CHECK(m.score() >= 24);

    // Check that plus atoms won't combine each other
    Model::atom expected = {0, Model::PLUS};
    size_t expected_length = m.get_length();
    // stick it at the edge so no worries about it combining early
    m.play_move(0); // + ...
    m.play_move(1); // + + ... (doesn't matter the order I put them)
    m.play_move(0); // + + + ...
    // all of them are still there and haven't combined
    CHECK(m.board()[0] == expected);
    CHECK(m.board()[1] == expected);
    CHECK(m.board()[2] == expected);
    CHECK(m.get_length() == expected_length + 3);
}

TEST_CASE("Plus Atom Kept Inside Circle Then Combines")
{
    Model m;
    // N, N, N, N, Plus, N / Plus, N
    std::vector<int> element_type = {10, 10, 10, 10, 21, 10, 21, 10};
    // H, He, Li, Be, Be / He
    std::vector<int> element_num = {0, 1, 2, 3, 3, 1};
    m.rand_atom_.stub_with(element_type);
    m.rand_element_.stub_with(element_num);

    m.play_move(0); // board: rand rand rand rand rand
    m.play_move(0); // board: H rand rand rand rand rand
    m.play_move(0); // board: He H rand rand rand rand rand
    m.play_move(2); // board: He H Li rand rand rand rand rand
    m.play_move(2); // board: He H Be Li rand rand rand rand rand
    // show that it waits
    Model::atom expected = {0, Model::PLUS};
    m.play_move(2); // board: He H + Be Li rand rand rand rand rand
    CHECK(m.board()[2] == expected);
    CHECK(m.get_length() == 10);
    CHECK(m.score() == 0);
    m.play_move(2); // board: He H Be + Be Li rand rand rand rand rand
    // Reduces exactly once
    CHECK_FALSE(m.board()[3] == expected);
    CHECK(m.get_length() == 9);
    // Get 7 (floor of (4+1) * 1.5) points for combining Be Be
    CHECK(m.score() == 7);

    // board now: He H B Li rand rand rand rand rand
    // put plus on edge:
    m.play_move(0); // board: + He H B Li rand rand rand rand rand
    // check that it's waiting
    CHECK(m.board()[0] == expected);
    CHECK(m.get_length() == 10);
    CHECK(m.score() == 7);
    m.play_move(0); // board: He + He H B Li rand rand rand rand rand
    // check it combined
    CHECK(m.get_length() == 9);
    expected = {3, Model::NORM};
    // now should have a Li at the 0th place
    CHECK(m.board()[0] == expected);
    // Get 4 (floor of (2+1) * 1.5) more points for combining He He
    CHECK(m.score() == 11);

}

TEST_CASE("Minus Atom")
{
    // First Part: changing an atom to plus using minus
    Model m;
    // N, N, N, N, Minus, Minus, Plus, Minus
    std::vector<int> element_type = {10, 10, 10, 10, 26, 26, 21, 26};
    std::vector<int> element_num = {2, 2, 4, 4}; // Li, Li, B, B
    m.rand_atom_.stub_with(element_type);
    m.rand_element_.stub_with(element_num);

    m.play_move(0); // this one is random
    m.play_move(2); // board: rand rand Li rand rand rand

    // at this point, should have one lithium at 2 and lithium in the next_atom
    CHECK(m.board()[2].type == Model::NORM);
    CHECK(m.board()[2].element == 3);
    CHECK(m.get_current_atom().type == Model::NORM);
    CHECK(m.get_current_atom().element == 3);

    m.play_move(2); // board: rand rand Li Li rand rand rand
    m.play_move(2); // board: rand rand B Li Li rand rand rand
    m.play_move(5); // board: rand rand B Li Li B rand rand rand
    CHECK(m.get_length() == 9);

    Model::atom expected = {m.board()[0].element, Model::MINUS};
    m.play_move(0); // board: rand Li Li rand rand rand
    // show that an atom was taken out of the circle
    CHECK(m.get_length() == 8);
    // should have the first atom's element and MINUS in the current atom place
    CHECK(expected == m.get_current_atom());
    // turn that atom into plus with extra parameter special
    m.play_move(2, true); // index doesn't matter when special
    expected = {0, Model::PLUS};
    CHECK(expected == m.get_current_atom());
    CHECK(m.get_length() == 8);
    CHECK(m.score() == 0);

    // Placing plus atom
    m.play_move(3); // board: rand B Li + Li B rand rand rand
    // at most four random atoms + 1 combined atom --> at most 5 atoms
    CHECK(m.get_length() < 6);
    // should get at least 6 points for combining Li Li, then 18 for B B
    // (10 - 4 + 3) * 2 = 18
    CHECK(m.score() >= 24);

    // Second Part: moving an atom using minus
    size_t expected_length = m.get_length();
    expected = {0, Model::MINUS}; // confirm we have a minus coming up
    CHECK(expected == m.get_current_atom());
    expected = {m.board()[0].element, Model::MINUS};
    m.play_move(0); // should have the first atom's element and minus in the
    // current atom place
    CHECK(expected == m.get_current_atom());
    // move that atom to index 1
    expected.type = Model::NORM;
    m.play_move(1);
    // check that it moved correctly from index 0 to index 1
    CHECK(m.board()[1] == expected);
    // check that the length didn't change
    CHECK(m.get_length() == expected_length);

    // Third Part: minus atom only works on normal atoms
    m.play_move(0); // put the plus in play on the edge so it won't
    // accidentally combine before we use it to prove a point
    expected_length = m.get_length();
    m.play_move(0); // try to play the minus on the plus
    // plus didn't disappear
    expected = {0, Model::PLUS};
    CHECK(m.board()[0] == expected);
    // minus still in the current atom place, ready to be played
    expected = {0, Model::MINUS};
    CHECK(m.get_current_atom() == expected);
    // then it plays as usual on a different atom
    expected = {m.board()[1].element, Model::MINUS};
    m.play_move(1); // should have the first atom's element and minus in the
    // current atom place
    CHECK(expected == m.get_current_atom());
    expected.type = Model::NORM;
    m.play_move(2); // move that atom to index 2
    // check that it moved correctly from index 1 to index 2
    CHECK(m.board()[2] == expected);
    // check that the length didn't change
    CHECK(m.get_length() == expected_length);
    // check that plus is still there
    expected = {0, Model::PLUS};
    CHECK(m.board()[0] == expected);
}

TEST_CASE("Black Plus Atom")
{
    Model m;
    // overriding needing a high enough score to get a black plus
    Test_access t {m};
    t.change_b_plus_score_(0);

    // N, N, N, N, B_Plus, B_Plus, N
    std::vector<int> element_type = {10, 10, 10, 10, 29, 29, 10};
    std::vector<int> element_num = {2, 3, 4, 4, 3}; // Li, Be, B, B, Be
    m.rand_atom_.stub_with(element_type);
    m.rand_element_.stub_with(element_num);
    // all the rand atoms are normal
    m.play_move(0); // board: rand rand rand rand rand
    m.play_move(2); // board: rand rand Li rand rand rand
    m.play_move(2); // board: rand rand Be Li rand rand rand
    m.play_move(2); // board: rand rand B Be Li rand rand rand
    m.play_move(5); // board: rand rand B Be Li B rand rand rand
    CHECK(m.get_length() == 9);
    CHECK(m.score() == 0);
    m.play_move(4); // board: rand rand B Be black+ Li B rand rand rand

    // at most five random atoms + 1 combined atom --> at most 6 atoms
    CHECK(m.get_length() < 7);
    // 2 points for combining Be Li, element_num = 6, then + 14 (7*2) = 16
    CHECK(m.score() >= 16);
    // setting up for next
    int sc = m.score();

    // What about putting it on the edge?
    size_t expect_length = m.get_length();
    int el_num = m.board()[expect_length-1].element;
    // place the black+
    m.play_move(expect_length);
    // shows that no atoms were combined and it is waiting
    Model::atom bplus = {0, Model::B_PLUS};
    CHECK(m.board()[expect_length] == bplus);
    expect_length++;
    CHECK(m.get_length() == expect_length);
    // check score hasn't changed
    CHECK(m.score() == sc);
    // then place Be on the edge
    m.play_move(expect_length);
    // we know there can't be any chain reactions on the edge, so the
    // combination reduces the old length by 1; end of the board goes from
    // (rand black+) to just (combinedatom) so decreases length by 1 by playing
    // an atom on the edge to activate the black+
    expect_length--;
    CHECK(m.get_length() == expect_length);
    // must at least be that edge el_num + 3
    CHECK(m.board()[expect_length-1].element >= el_num + 3);
    // score must've increased
    CHECK(m.score() > sc);
}

TEST_CASE("Game ends after filling board")
{
    Model m;
    int element_type = 10; // all will be NORM
    m.rand_atom_.stub_with(element_type);
    // currently 4 random atoms in play, then add 16 more
    for (int i = 0; i < 16; i++) {
        CHECK_FALSE(m.is_game_over());
        m.play_move(0);
    }
    CHECK(m.is_game_over());
}
