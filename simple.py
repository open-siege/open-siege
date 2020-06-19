import darkstar

game = darkstar.currentInstance()

console = game.getConsole()

console.echo("Hello world from Python in simple.py")

class TestCallback(darkstar.PyConsoleCallback):
    def doExecuteCallback(self, cmd, callbackId, args):
        try:
            console.echo("Python has been summoned. What do you want from it?");
        except Exception as ex:
            with open("python-errors.log", "w") as errorLog:
                errorLog.write(str(ex))

     #   for arg in args:
     #       console.eval(f'echo("{arg}");')
        return "True"

console.echo("Adding a callback")

theCallback = TestCallback()

console.addCommand(0, "pythonTestCallback", theCallback, 0)

console.echo("Done adding a callback")

game.knownPlugins.terrain.saveTerrain("hello", "world")