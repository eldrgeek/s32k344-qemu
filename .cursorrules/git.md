---
name: "Git Operations"
description: "Version control rules and safe Git operations"
applies_to:
  - "**/*"
contexts:
  - "git"
  - "version-control"
  - "commits"
priority: 85
---

# Git Operations Rules

## Always Safe Git Commands

### Information and Status
- `git status` - Check repository state
- `git log` - View commit history
- `git diff` - See changes
- `git show` - Show commit details
- `git branch` - List branches
- `git remote -v` - Check remotes

### Safe Operations
- `git add` - Stage changes (always safe)
- `git commit` - Commit staged changes (always safe)
- `git push` - Push to remote (safe for feature branches)
- `git pull` - Pull from remote (safe for feature branches)
- `git checkout <file>` - Restore file (safe when intentional)
- `git stash` - Stash changes (safe backup)

## Commit Strategy

### Atomic Commits
Each commit should represent one logical change:
- **feat**: New feature implementation
- **fix**: Bug fixes
- **test**: Adding or updating tests
- **docs**: Documentation updates
- **build**: Build system changes
- **ci**: CI/CD changes
- **refactor**: Code restructuring
- **style**: Code formatting

### Commit Message Examples
```
feat(lpuart): implement register read/write operations
fix(siul2): correct GPIO pin mapping for S32K344
test(qtest): add LPUART transmission test
docs(readme): update build instructions for Docker
build(qemu): integrate S32K344 into QEMU build system
```

### Branch Strategy
- `main` - Always stable, ready for release
- `feature/s32k344` - Main development branch
- `feature/lpuart-impl` - Specific feature branches
- `hotfix/critical-fix` - Emergency fixes

### Safe Git Workflows

#### Feature Development
```bash
# Start feature
git checkout -b feature/new-peripheral
git add .
git commit -m "feat(peripheral): initial implementation"

# Regular commits
git add file.c
git commit -m "feat(peripheral): add register mapping"

# Push feature
git push -u origin feature/new-peripheral
```

#### Integration
```bash
# Stage all changes
git add -A

# Commit milestone
git commit -m "feat(s32k344): complete LPUART integration

- Implement LPUART register read/write operations
- Add LPUART to S32K344 machine initialization
- Update build configuration for new peripheral
- Add basic QTest validation for LPUART
"

# Push to feature branch
git push origin feature/s32k344
```

### File Management
Always safe to add these patterns:
- Source code: `src/**/*.c`, `src/**/*.h`
- Tests: `tests/**/*.c`
- Documentation: `docs/**/*.md`
- Build configs: `meson.build`, `Makefile`
- Project files: `*.md`, `*.yml`, `*.sh`

### Ignore Patterns
Already configured in `.gitignore`:
- Build artifacts: `build/`, `*.o`, `*.a`
- IDE files: `.vscode/`, `*.swp`
- OS files: `.DS_Store`, `Thumbs.db`
- Virtual environments: `venv/`, `__pycache__/` 