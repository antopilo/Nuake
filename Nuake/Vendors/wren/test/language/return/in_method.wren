class Foo {
  construct new() {}

  method {
    return "ok"
    System.print("bad")
  }
}

System.print(Foo.new().method) // expect: ok
