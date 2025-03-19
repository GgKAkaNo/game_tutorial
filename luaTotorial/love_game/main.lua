function love.load()
    image = love.graphics.newImage("assets/image1.jpg")
end

function love.update(dt)
    
end

function love.draw()
    love.graphics.draw(image, 100, 0)
    --love.graphics.draw(image, love.math.random(0, 800), love.math.random(0, 600))
end

function love.run()
    if love.math then
        love.math.setRandomSeed(os.time())
    end

    if love.load then love.load(arg) end

    if love.timer then love.timer.step() end

    local dt = 0

    -- Main loop time
    while true do
        -- process events
        if love.event then
            love.event.pump()
            for name, a, b, c, d, e, f in love.event.poll() do
                if name == "quit" then
                    if not love.quit or not love.quit() then
                        return a
                    end
                end
                love.handlers[name](a, b, c, d, e, f)
            end
        end

        -- update dt, as we will be passing it to update
        if love.timer then
            love.timer.step()
            dt = love.timer.getDelta()
        end

        -- call update and draw
        if love.update then love.update(dt) end
        
        if love.graphics and love.graphics.isActive() then
            love.graphics.clear(love.graphics.getBackgroundColor())
            love.graphics.origin()

            if love.draw then love.draw() end
            love.graphics.present()
        end

        
        if love.timer then love.timer.sleep(0.001) end
    end
    
end
