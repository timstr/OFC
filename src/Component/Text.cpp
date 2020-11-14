#include <OFC/Component/Text.hpp>

namespace ofc::ui {

    namespace detail {
    
        std::vector<String> splitIntoWords(const String& s) {
            std::vector<String> out;
            String word;
            auto pushWord = [&](){
                if (word.getSize() > 0) {
                    out.push_back(word);
                    word.clear();
                }
            };
            for (const auto& c : s){
                if (c == U'\n'){
                    // TODO: line break
                    pushWord();
                } else if (c == U'\t'){
                    // TODO: tab
                    pushWord();
                } else if (c == U' '){
                    pushWord();
                } else {
                    word += c;
                }
            }
            pushWord();
            return out;
        }

    } // namespace detail

    Text::Text(Valuelike<String> s)
        : FontConsumer(&Text::updateFont)
        , m_stringObserver(this, &Text::updateString, std::move(s)) {

    }

    std::unique_ptr<dom::Text> Text::createElement() {
        return std::make_unique<dom::Text>(
            m_stringObserver.getValuelike().getOnce(),
            *getFont().getValuelike().getOnce(),
            sf::Color::Black
        );
    }

    void Text::updateString(const String& s) {
        element()->setText(s);
    }

    void Text::updateFont(const sf::Font* f) {
        assert(f);
        element()->setFont(*f);
    }
    
    

    Span::Span(Valuelike<String> s)
        : FontConsumer(&Span::updateFont)
        // TODO: make it so that calling s.map(...) does the right thing here
        , m_wordsObserver(this, &Span::updateWords, combine(std::move(s)).map(detail::splitIntoWords)) {

    }

    void Span::onMount(const dom::Element* beforeSibling) {
        const auto& newWords = m_wordsObserver.getValuelike().getOnce();
        m_words.reserve(newWords.size());
        for (const auto& w : newWords) {
            auto t = makeWord(w);
            auto tp = t.get();
            insertElement(std::move(t), beforeSibling);
            m_words.push_back(tp);
        }
    }

    void Span::onUnmount() {
        for (auto t : m_words){
            eraseElement(t);
        }
    }

    void Span::updateWords(const ListOfEdits<String>& le) {
        m_words.reserve(le.newValue().size());
        auto it = begin(m_words);
        for (const auto& edit : le.getEdits()) {
            if (edit.deletion()){
                assert(it != end(m_words));
                eraseElement(*it);
                it = m_words.erase(it);
            } else if (edit.insertion()){
                dom::Element const* nextElement = nullptr;
                if (it == end(m_words)){
                    if (auto nc = getNextMountedComponent()) {
                        nextElement = nc->getFirstElement();
                    }
                } else {
                    nextElement = *it;
                }
                auto t = makeWord(edit.value());
                auto tp = t.get();
                insertElement(std::move(t), nextElement);
                it = m_words.insert(it, tp);
                ++it;
            } else if (edit.nothing()){
                assert(it != end(m_words));
                ++it;
            }
        }
    }

    void Span::updateFont(const sf::Font* f) {
        for (const auto& w : m_words) {
            assert(w);
            w->setFont(*f);
        }
    }

    std::unique_ptr<dom::Text> Span::makeWord(const String& str){
        return std::make_unique<dom::Text>(
            str,
            *getFont().getValuelike().getOnce(),
            sf::Color::Black
        );
    }
    
} // namespace ofc::ui
