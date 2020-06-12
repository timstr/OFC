#include <GUI/DOM/Image.hpp>

namespace ui::dom {

    Image::Image(const std::string& path, bool autoSize){
        if (!loadFromFile(path, autoSize)){
            throw std::runtime_error("Failed to load file for Image");
        }
    }

    Image::Image(const sf::Image& img, bool autoSize){
        if (!copyFromImage(img, autoSize)){
            throw std::runtime_error("Failed to copy from image");
        }
    }

    Image::Image(std::shared_ptr<sf::Texture> texture, bool autoSize ){
        if (!setTexture(std::move(texture), autoSize)){
            throw std::runtime_error("Failed to assign texture");
        }
    }

    sf::Texture& Image::getTexture(){
        assert(m_texture);
        return *m_texture;
    }
    const sf::Texture& Image::getTexture() const {
        assert(m_texture);
        return *m_texture;
    }

    std::shared_ptr<sf::Texture> Image::getTexturePtr() {
        return m_texture;
    }
    std::shared_ptr<const sf::Texture> Image::getTexturePtr() const {
        return m_texture;
    }

    bool Image::loadFromFile(const std::string& path, bool autoSize){
        if (!m_texture){
            m_texture = std::make_shared<sf::Texture>();
        }
        if (!m_texture->loadFromFile(path)){
            return false;
        }
        if (m_texture->getSize().x == 0 || m_texture->getSize().y == 0){
            return false;
        }
        m_sprite.setTexture(*m_texture, true);
        if (autoSize){
            const auto s = m_texture->getSize();
            setSize({static_cast<float>(s.x), static_cast<float>(s.y)}, true);
        }
        return true;
    }

    bool Image::copyFromImage(const sf::Image& img, bool autoSize){
        if (img.getSize().x == 0 || img.getSize().y == 0){
            return false;
        }
        m_texture = std::make_shared<sf::Texture>();
        m_texture->loadFromImage(img);
        m_sprite.setTexture(*m_texture, true);
        if (autoSize){
            const auto s = m_texture->getSize();
            setSize({static_cast<float>(s.x), static_cast<float>(s.y)}, true);
        }
        return true;
    }

    bool Image::setTexture(std::shared_ptr<sf::Texture> texture, bool autoSize){
        if (!texture){
            return false;
        }
        m_sprite.setTexture(*m_texture, true);
        if (autoSize){
            const auto s = m_texture->getSize();
            setSize({static_cast<float>(s.x), static_cast<float>(s.y)}, true);
        }
        return true;
    }

    void Image::setAlpha(uint8_t alpha){
        auto c = m_sprite.getColor();
        c.a = alpha;
        m_sprite.setColor(c);
    }

    uint8_t Image::alpha() const {
        return m_sprite.getColor().a;
    }

    void Image::setColorMod(Color color){
        m_sprite.setColor(color);
    }

    Color Image::colorMod() const {
        return m_sprite.getColor();
    }

    void Image::onResize(){
        const auto s = m_texture->getSize();
        m_sprite.setScale({
            width() / static_cast<float>(s.x),
            height() / static_cast<float>(s.y)
        });
    }

    void Image::render(sf::RenderWindow& rw){
        assert(m_sprite.getTexture());
        assert(m_sprite.getTexture() == m_texture.get());
        rw.draw(m_sprite);
    }

} // namespace ui::dom
