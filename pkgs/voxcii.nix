{
  stdenv,
  lib,
  ncurses,
}:

stdenv.mkDerivation {
  pname = "voxcii";
  version = "0-unstable-2026-03-07";

  # Supplied by the flake overlay so this derivation remains reusable.
  src = null;

  buildInputs = [ ncurses ];

  installPhase = ''
    runHook preInstall
    install -Dm755 voxcii "$out/bin/voxcii"
    runHook postInstall
  '';

  meta = {
    description = "Terminal-based ASCII 3D model viewer";
    homepage = "https://github.com/TheCodedKid/voxcii";
    license = lib.licenses.mit;
    mainProgram = "voxcii";
    platforms = lib.platforms.unix;
  };
}
