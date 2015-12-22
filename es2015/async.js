function ping() {
 return __awaiter(this, void 0, Promise, function* () {
  for (var i = 0; i < 10; i++) {
   yield delay(300);
   console.log("ping");
  }
 });
}
ping();